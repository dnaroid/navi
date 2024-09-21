#include <BootManager.h>
#include <CST816S.h>
#include <esp_wifi.h>
#include <globals.h>
#include <Mirror.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
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
extern Location my_gps_location;
extern CST816S touch;

static data_struct touch_data;
static std::vector<RouteExt> route = {};
static auto sprite = TFT_eSprite(&tft);
static bool camEnabled = false;
static int camWsClientNumber = -1;
static int mirror_width;
static int mirror_height;
static float distance = -1;
static int zoom = ZOOM_TRIP;
static WebSocketsServer webSocket(81);
static bool serverReady = false;
static bool spriteReady = false;
static Location my_location;
static int nextTurnAngle = 0;
static float nextTurnDistance = -1;
// cache
static int mirror_center_x;
static int mirror_center_y;
static lv_point_t screenCenter;
static lv_point_t centerPx;
static float rad_angle;
static float cosAngle;
static float sinAngle;
static float scale;
static char distanceText[20] = "\0";
static char turnText[20] = "\0";

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

static void drawArrow(int32_t x, int32_t y, int32_t r, int angle) {
  angle = max(min(angle, 180), -180);
  int startDeg = angle > 0 ? 90 : 270 + angle;
  int endDeg = angle > 0 ? 90 + angle : 270;
  int arrow_w = 5;
  float arrow_l = angle > 0 ? -.5 : .5;

  sprite.drawArc(x, y, r + 1, r - 1, startDeg, endDeg, TFT_WHITE, TFT_BLACK);

  float endAngleRad = (angle > 0 ? 180 - angle : 180 - (180 + angle)) * PI / 180;

  float cosEndAngleRad = cos(endAngleRad);
  float sinEndAngleRad = sin(endAngleRad);

  int arrowTipX = x + r * cos(endAngleRad + arrow_l);
  int arrowTipY = y - r * sin(endAngleRad + arrow_l);

  int arrowBaseX1 = x + (r - arrow_w) * cosEndAngleRad;
  int arrowBaseY1 = y - (r - arrow_w) * sinEndAngleRad;

  int arrowBaseX2 = x + (r + arrow_w) * cosEndAngleRad;
  int arrowBaseY2 = y - (r + arrow_w) * sinEndAngleRad;

  sprite.fillTriangle(arrowTipX, arrowTipY, arrowBaseX1, arrowBaseY1, arrowBaseX2, arrowBaseY2, TFT_WHITE);
}

static void drawText() {
  sprite.drawString(distanceText, 50, btn_zoom_out.y);

  if (nextTurnDistance > 0) {
    sprite.drawString(turnText, 0, btn_exit.y);
    drawArrow(17, 17, 10, nextTurnAngle);
  }
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

static void drawRoute() {
  const auto angle_fixed = -static_cast<int16_t>((static_cast<int>(compass_angle) + COMPASS_ANGLE_CORRECTION) % 360);
  rad_angle = angle_fixed * PI / 180.0;
  cosAngle = cos(rad_angle);
  sinAngle = sin(rad_angle);

  for (auto& i : route) i.px = globalToScreenCoords(i.point);

  for (int i = 0; i < route.size() - 1; ++i) {
    int x1 = route[i].px.x;
    int y1 = route[i].px.y;
    int x2 = route[i + 1].px.x;
    int y2 = route[i + 1].px.y;
    float distance = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
    int numCircles = distance / 10;
    float dx = (x2 - x1) / distance;
    float dy = (y2 - y1) / distance;

    for (int j = 0; j <= numCircles; ++j) {
      int x = x1 + dx * (j * 10);
      int y = y1 + dy * (j * 10);
      sprite.fillCircle(x, y, 2, UI_COLOR);
      sprite.drawCircle(x, y, 3, UI_COLOR2);
    }
  }
}

#define ANGLE_TOLERANCE     45
#define MAX_DISTANCE_M   500.0

static void findNextTurnIdx() {
  nextTurnAngle = 0;
  nextTurnDistance = -1.0;
  for (int i = 0; i < route.size(); ++i) {
    nextTurnDistance += route[i].distance;
    if (nextTurnDistance > MAX_DISTANCE_M) {
      nextTurnDistance = -1.0;
      break;
    };
    if (abs(route[i].angle) >= ANGLE_TOLERANCE) {
      nextTurnAngle = route[i].angle;
      break;
    };
  }
}

static void updateMyLocation(Location new_location) {
  my_location = new_location;
  centerPx = mercatorProjection(my_location);

  updateRouteExtProgress(my_location, route);

  distance = 0;
  for (const auto r : route) distance += r.distance;

  if (distance < 1000.0) {
    snprintf(distanceText, sizeof(distanceText), " %d m ", static_cast<int>(distance));
  } else {
    snprintf(distanceText, sizeof(distanceText), " %.1f km ", distance / 1000);
  }

  findNextTurnIdx();
  if (nextTurnDistance > 0) {
    snprintf(turnText, sizeof(turnText), "       %dm ", static_cast<int>(nextTurnDistance));
  }
}

#define GPS_TOLERANCE 0.002

static void checkGpsLocation() {
  if (my_gps_location.lat == 0 || simpleDistance(my_gps_location, my_location) < GPS_TOLERANCE) return;
  updateMyLocation(my_gps_location);
}


static void updateUI() {
  // auto ms = millis();
  drawRoute();
  drawMarker();
  drawButtons();
  drawText();
  checkGpsLocation();
  // LOG(millis() - ms);
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
    // sprite.fillScreen(TFT_BLACK); // todo debug
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

static void processRoute(const std::vector<Location>& rawRoute) {
  for (int i = 0; i < rawRoute.size(); i++) {
    route.resize(rawRoute.size());
    route[i].point = rawRoute[i];
    if (i + 1 < rawRoute.size()) {
      route[i].distance = getDistanceMeters(rawRoute[i], rawRoute[i + 1]);
    }
    if (i + 2 < rawRoute.size()) {
      route[i].angle = calculateAngle(rawRoute[i], rawRoute[i + 1], rawRoute[i + 2]);
    }
    // LOG(route[i].distance, route[i].angle);
  }
}

static Location DEBUG_moveOneMeterTowardsNextPoint(const Location& p1, const Location& p2) {
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
    if (simpleDistance(route[0].point, route[1].point) < 0.002) { // todo remove debug
      updateMyLocation(DEBUG_moveOneMeterTowardsNextPoint(route[1].point, route[2].point)); // todo remove debug
    } else { // todo remove debug
      updateMyLocation(DEBUG_moveOneMeterTowardsNextPoint(route[0].point, route[1].point)); // todo remove debug
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
  pinMode(MIRROR_POWER_PIN,OUTPUT);

  processRoute(state.route);
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
  if (!serverReady) Mirror_server_init();
  digitalWrite(MIRROR_POWER_PIN,HIGH);
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

