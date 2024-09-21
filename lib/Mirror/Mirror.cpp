#include <BootManager.h>
#include <CST816S.h>
#include <esp_wifi.h>
#include <globals.h>
#include <Mirror.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include <Touch.h>
#include <misc/lv_area.h>
#include "WebSocketsServer.h"
#include "UIHelpers.h"

#define MARKER_SIZE   20
#define TAP_ZONE      50
#define UI_COLOR      TFT_BLACK
#define UI_COLOR2     TFT_WHITE

lv_point_t btn_exit;
lv_point_t btn_zoom_in;
lv_point_t btn_zoom_out;

extern SemaphoreHandle_t xWireSemaphore;
extern TFT_eSPI tft;
extern CST816S touch;
data_struct touch_data;
NextTurn next_turn = {false, -1};

auto sprite = TFT_eSprite(&tft);
bool camEnabled = false;
int camWsClientNumber = -1;
static int mirror_width;
static int mirror_height;
static std::vector<Location> route = {};
static lv_point_t* route_px;
static float distance = -1;
static int zoom = ZOOM_TRIP;
static WebSocketsServer webSocket(81);
static bool serverReady = false;
static bool spriteReady = false;
static Location my_location;
// cache
static int mirror_center_x;
static int mirror_center_y;
static lv_point_t screenCenter;
static lv_point_t centerPx;
float rad_angle;
float cosAngle;
float sinAngle;
float scale;
char distanceText[20] = "\0";
char turnText[20] = "\0";

static lv_point_t mercatorProjection(Location loc) {
  int x = (loc.lon + 180.0) / 360.0 * scale;
  int y = (1.0 - log(tan(loc.lat * PI / 180.0) + 1.0 / cos(loc.lat * PI / 180.0)) / PI) / 2.0 * scale;
  return {x, y};
}

static lv_point_t rotatePoint_(lv_point_t point) {
  lv_point_t rotated;
  rotated.x = cosAngle * (point.x - screenCenter.x) - sinAngle * (point.y - screenCenter.y) + screenCenter.x;
  rotated.y = sinAngle * (point.x - screenCenter.x) + cosAngle * (point.y - screenCenter.y) + screenCenter.y;
  return rotated;
}

static lv_point_t globalToScreenCoords(Location point) {
  lv_point_t pointPx = mercatorProjection(point);
  return rotatePoint_({screenCenter.x + (pointPx.x - centerPx.x), screenCenter.y + (pointPx.y - centerPx.y)});
}

void drawArrow(int32_t x, int32_t y, int32_t r, int angle) {
  int32_t startDeg;
  int32_t endDeg;
  int arrow_size = 4;

  if (angle > 0) {
    angle = min(angle, 180);
    startDeg = 90;
    endDeg = 90 + angle;
  } else {
    return;
  }

  sprite.drawArc(x, y, r, r - 2, startDeg % 360, endDeg % 360, TFT_WHITE, TFT_BLACK);

  float endAngleRad = (180 - angle) * PI / 180;

  int arrowTipX = x + (r - 1) * cos(endAngleRad - .3);
  int arrowTipY = y - (r - 1) * sin(endAngleRad - .3);

  int arrowBaseX1 = x + (r - 1 - arrow_size) * cos(endAngleRad);
  int arrowBaseY1 = y - (r - 1 - arrow_size) * sin(endAngleRad);

  int arrowBaseX2 = x + (r - 1 + arrow_size) * cos(endAngleRad);
  int arrowBaseY2 = y - (r - 1 + arrow_size) * sin(endAngleRad);

  sprite.fillTriangle(arrowTipX, arrowTipY, arrowBaseX1, arrowBaseY1, arrowBaseX2, arrowBaseY2, TFT_WHITE);
}

static void drawText() {
  sprite.drawString(distanceText, mirror_center_x - 25, btn_zoom_out.y);

  if (next_turn.distance < 0) return;

  sprite.drawString(turnText, 0, btn_exit.y);

  drawArrow(30, 100, 20, 45);
  drawArrow(150, 100, 20, 90);
  drawArrow(30, 200, 20, 127);
  drawArrow(150, 200, 20, 180);
}

static void drawButtons() {
  sprite.drawString(" x ", btn_exit.x, btn_exit.y);
  sprite.drawString(" + ", btn_zoom_in.x, btn_zoom_in.y);
  sprite.drawString(" - ", btn_zoom_out.x, btn_zoom_out.y);
}

static void drawMarker() {
  sprite.fillTriangle(mirror_center_x, mirror_center_y,
                      mirror_center_x - MARKER_SIZE / 3, mirror_center_y + MARKER_SIZE,
                      mirror_center_x + MARKER_SIZE / 3, mirror_center_y + MARKER_SIZE,
                      UI_COLOR);
  sprite.drawTriangle(mirror_center_x, mirror_center_y,
                      mirror_center_x - MARKER_SIZE / 3, mirror_center_y + MARKER_SIZE,
                      mirror_center_x + MARKER_SIZE / 3, mirror_center_y + MARKER_SIZE,
                      UI_COLOR2);
}

static void updateRoute() {
  const auto angle_fixed = -static_cast<int16_t>((static_cast<int>(compass_angle) + COMPASS_ANGLE_CORRECTION) % 360);
  rad_angle = angle_fixed * PI / 180.0;
  cosAngle = cos(rad_angle);
  sinAngle = sin(rad_angle);
  for (int i = 0; i < route.size(); i++) {
    route_px[i] = globalToScreenCoords(route[i]);
  }
  for (int i = 0; i < route.size() - 1; ++i) {
    sprite.drawLine(route_px[i].x, route_px[i].y, route_px[i + 1].x, route_px[i + 1].y, UI_COLOR);
    int deltaX = route_px[i + 1].x - route_px[i].x;
    int deltaY = route_px[i + 1].y - route_px[i].y;
    lv_point_t offsetA = route_px[i];
    lv_point_t offsetB = route_px[i + 1];
    if (abs(deltaX) > abs(deltaY)) {
      offsetA.y += 1;
      offsetB.y += 1;
    } else if (abs(deltaX) < abs(deltaY)) {
      offsetA.x -= 1;
      offsetB.x -= 1;
    }
    sprite.drawLine(offsetA.x, offsetA.y, offsetB.x, offsetB.y, UI_COLOR2);
  }
}

static void updateUI() {
  updateRoute();
  drawMarker();
  drawButtons();
  drawText();
}

static void updateMyLocation(Location new_location) {
  my_location = new_location;
  centerPx = mercatorProjection(my_location);

  updateRouteProgress(my_location, route);

  distance = getRouteDistance(route);
  if (distance < 1.0) {
    const int meters = static_cast<int>(distance * 1000);
    snprintf(distanceText, sizeof(distanceText), " %d m ", meters);
  } else {
    snprintf(distanceText, sizeof(distanceText), " %.1f km ", distance);
  }

  next_turn = getDistanceToNextTurn(route);
  if (next_turn.distance > 0) {
    snprintf(turnText, sizeof(turnText), "     %dm ", next_turn.distance);
  }
}

static void initRenderer() {
  mirror_center_x = mirror_width / 2;
  mirror_center_y = mirror_height / 2;

  sprite.createSprite(mirror_width, mirror_height);
  sprite.setTextFont(4);
  sprite.setTextColor(UI_COLOR2, TFT_BLACK);
  spriteReady = true;

  screenCenter = {mirror_center_x, mirror_center_y};
  scale = TILE_SIZE * pow(2, zoom); // todo refresh on zoom  changed
  centerPx = mercatorProjection(my_location); // todo refresh on zoom && location changed

  btn_exit = {mirror_width - 20, 0};
  btn_zoom_in = {btn_exit.x, mirror_height - 20};
  btn_zoom_out = {0, btn_zoom_in.y};

  updateMyLocation(my_location);
}

static bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  sprite.pushImage(x, y, w, h, bitmap);
  if (x + w >= mirror_width && y + h >= mirror_height) {
    sprite.fillScreen(TFT_BLACK); // todo drbug
    updateUI();
    if (xSemaphoreTake(xWireSemaphore, portMAX_DELAY) == pdTRUE) {
      sprite.pushSprite(SCREEN_CENTER_X - mirror_width / 2, SCREEN_CENTER_Y - mirror_height / 2);
      xSemaphoreGive(xWireSemaphore);
    }
  }
  return true;
}

static void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  int commaIndex;
  String receivedData;
  switch (type) {
  case WStype_DISCONNECTED:
    LOGF("[%u] Disconnected\n", num);
    camWsClientNumber = -1;
    break;
  case WStype_CONNECTED:
    LOGF("[%u] Connected\n", num);
    camWsClientNumber = num;
    if (camEnabled) {
      webSocket.sendTXT(num, "start");
    } else {
      webSocket.sendTXT(num, "stop");
    }
    break;
  case WStype_BIN:
    if (spriteReady) TJpgDec.drawJpg(0, 0, payload, length);
    break;
  case WStype_TEXT:
    receivedData = String((char*)payload);
    commaIndex = receivedData.indexOf(',');
    if (commaIndex != -1) {
      String widthStr = receivedData.substring(0, commaIndex);
      String heightStr = receivedData.substring(commaIndex + 1);
      mirror_width = widthStr.toInt();
      mirror_height = heightStr.toInt();
      initRenderer();
    }
    break;
  default:
    break;
  }
}

static void onPressExit() {
  if (camWsClientNumber > 0) webSocket.sendTXT(camWsClientNumber, "stop");
  delay(100);
  switchBootMode(ModeMap);
  esp_restart();
}

static void onPressZoomIn() {
  zoom++;
  scale = TILE_SIZE * pow(2, zoom);
  updateMyLocation(my_location);
}

static void onPressZoomOut() {
  zoom--;
  scale = TILE_SIZE * pow(2, zoom);
  updateMyLocation(my_location);
}

Location DEBUG_moveOneMeterTowardsNextPoint(const Location& p1, const Location& p2) {
  float dLat = p2.lat - p1.lat;
  float dLon = (p2.lon - p1.lon) * cos(p1.lat * M_PI / 180.0);
  float distance = sqrt(dLat * dLat + dLon * dLon) * 111320;
  float ratio = 1.0 / distance;
  float moveLat = dLat * ratio;
  float moveLon = dLon * ratio;
  Location newLocation;
  newLocation.lat = p1.lat + moveLat;
  newLocation.lon = p1.lon + moveLon;
  return newLocation;
}

static void onClick() {
  int x = touch.data.x;
  int y = touch.data.y;

  if (x > btn_exit.x - TAP_ZONE && y < btn_exit.y + TAP_ZONE) {
    onPressExit();
  } else if (x > btn_zoom_in.x - TAP_ZONE && y > btn_zoom_in.y - TAP_ZONE && zoom < ZOOM_MAX) {
    onPressZoomIn();
  } else if (x < btn_zoom_out.x + TAP_ZONE && y > btn_zoom_out.y - TAP_ZONE && zoom > ZOOM_MIN) {
    onPressZoomOut();
  } else { // todo remove debug
    if (simpleDistance(route[0], route[1]) < 0.002) { // todo remove debug
      updateMyLocation(DEBUG_moveOneMeterTowardsNextPoint(route[1], route[2])); // todo remove debug
    } else { // todo remove debug
      updateMyLocation(DEBUG_moveOneMeterTowardsNextPoint(route[0], route[1])); // todo remove debug
    } // todo remove debug
  } // todo remove debug
}

void Mirror_server_init() {
  esp_wifi_set_max_tx_power(1);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  LOGI("WS server IP:", WiFi.softAPIP());
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  serverReady = true;
}

void Mirror_init(const BootState& state) {
  LOGI("Init Mirror");
  route = state.route;
  route_px = new lv_point_t[state.route.size()];
  distance = state.distance;
  my_location = state.start;

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  Mirror_server_init();

  touch.begin();

  LOG(" ok");
}

void Mirror_start() {
  if (!serverReady) { Mirror_server_init(); }
  camEnabled = true;
  LOG("[Mirror.cpp] cam enabled");
  if (camWsClientNumber != -1) webSocket.sendTXT(camWsClientNumber, "start");
}

void Mirror_loop() {
  if (serverReady) webSocket.loop();
  // if (touch.available() && (touch.data.gestureID == SINGLE_CLICK)) onClick();
  // todo replace debug
  if (touch.available() && (touch.data.gestureID == SINGLE_CLICK || touch.data.gestureID == LONG_PRESS)) onClick();
}

