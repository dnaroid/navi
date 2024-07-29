#include "globals.h"
#include <PNGdec.h>
#include <SD.h>
#include <SPI.h>
#include <TinyGPSPlus.h>
#include <Wire.h>

#include <TFT_eSPI.h>
#include <WiFi.h>

#include "coord.h"
#include "png.h"
#include "LSM303.h"
#include "UI.h"
#include "Touch.h"
#include "Address.h"
#include "Router.h"

auto spiDisplay = SPIClass(HSPI);
auto spiSD = SPIClass(VSPI);

TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

LSM303 compass;
UI ui;
Touch touch;
Address address;
Router router;

// global vars
int zoom = 14;
int angle = 0;
int new_angle = 0;
float init_lat = 54.3926814;
float init_lon = 18.6209547;
Location centerLoc = {init_lon, init_lat};
Location targetLoc = {0, 0};
Location myLoc = centerLoc;
unsigned int now;
bool isKeyboardActive = false;
bool isShowAddresses = false;
unsigned long compassUpdateAfterMs;
unsigned long gpsUpdateAfterMs;

static char path_name[MAX_FILENAME_LENGTH];

const char* getTilePath(int z, int x, int y) {
  std::snprintf(path_name, sizeof(path_name), "/tiles/%d/%d/%d.png", z, x, y);
  return path_name;
}

void drawTile(int x, int y, int sx, int sy) {
  int x2 = sx + TILE_SIZE;
  int y2 = sy + TILE_SIZE;
  if (sx <= SCREEN_WIDTH && x2 >= 0 && sy <= SCREEN_HEIGHT && y2 >= 0) {
    drawPngTile(getTilePath(zoom, x, y), sx, sy);
  }
}

void drawMyMarker() {
  if (!myLoc.lat) { return; }
  Point p = coord::locationToScreen(myLoc, centerLoc, zoom);
  TFT.fillCircle(p.x, p.y, MY_MARKER_R,TFT_BLUE);
  float angle_rad = angle * DEG_TO_RAD;
  int offsetX = (MY_MARKER_R - MY_MARKER_R2 - 1) * cos(angle_rad);
  int offsetY = -(MY_MARKER_R - MY_MARKER_R2 - 1) * sin(angle_rad);
  TFT.fillCircle(p.x + offsetX, p.y + offsetY, MY_MARKER_R2, TFT_WHITE);
}

void drawTargetMarker() {
  if (!targetLoc.lon) { return; }
  Point p = coord::locationToScreen(targetLoc, centerLoc, zoom);
  TFT.fillCircle(p.x, p.y, MY_MARKER_R,TFT_GREEN);
}

#define circleRadius 5
#define gapPixels 10

void drawRoute() {
  const std::vector<Location> route = router.getRoute();
  if (route.empty()) { return; }
  Point p1 = coord::locationToScreen(route[0], centerLoc, zoom);
  TFT.fillCircle(p1.x, p1.y, circleRadius, TFT_BLUE);
  for (size_t i = 1; i < route.size(); i++) {
    const Point p2 = coord::locationToScreen(route[i], centerLoc, zoom);
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    float distance = sqrt(dx * dx + dy * dy);
    // Расставляем дополнительные круги, если расстояние между точками больше, чем зазор
    if (distance > (circleRadius * 2 + gapPixels)) {
      int numAdditionalCircles = (int)((distance - circleRadius * 2) / (circleRadius * 2 + gapPixels));
      for (int j = 1; j <= numAdditionalCircles; j++) {
        float newX = p1.x + j * ((dx / numAdditionalCircles));
        float newY = p1.y + j * ((dy / numAdditionalCircles));
        TFT.fillCircle(newX, newY, circleRadius, TFT_BLUE);
      }
    }
    p1 = p2;
    TFT.fillCircle(p1.x, p1.y, 5, TFT_BLUE);
    TFT.drawLine(p1.x, p1.y, p2.x, p2.y,TFT_BLUE);
  }
}

void drawMap() {
  Point p = coord::locationToPixels(centerLoc, zoom);
  int x_tile = p.x / TILE_SIZE;
  int y_tile = p.y / TILE_SIZE;
  int x_offset = p.x % TILE_SIZE;
  int y_offset = p.y % TILE_SIZE;
  int tile_screen_x = SCREEN_CENTER_X - x_offset;
  int tile_screen_y = SCREEN_CENTER_Y - y_offset;
  for (int dx : TILES_X_SCAN) {
    for (int dy : TILES_Y_SCAN) {
      drawTile(x_tile + dx, y_tile + dy, tile_screen_x + dx * TILE_SIZE, tile_screen_y + dy * TILE_SIZE);
    }
  }
  drawMyMarker();
  drawTargetMarker();
  drawRoute();
}

void showAddresses() {
  int id = 0;
  for (const auto res : address.getResults()) {
    ui.findButtonById(id++).caption(res.name).visible(true);
  }
  ui.update();
}

void toggleKeyboard() {
  isKeyboardActive = !isKeyboardActive;
  if (!isKeyboardActive) {
    drawMap();
  } else {
    ui.findInputById('a').clear();
    ui.toggleBtnByType('a', false);
  }
  ui.findInputById('a').visible(isKeyboardActive);
  ui.toggleBtnByType('k', isKeyboardActive);
  ui.update();
}

void onAddresPressed(const Button& btn) {
  centerLoc = address.getResults()[btn.id].location;
  targetLoc = centerLoc;
  drawMap();
  for (int i = 0; i < ADDR_SEARCH_LIMIT; i++) ui.findButtonById(i).caption("").visible(false);
  ui.update();
}

void onZoomBtnPressed(Button& btnZoom) {
  ui.update();
  zoom += btnZoom.text == "+" ? 1 : -1;
  ui.findButtonByText("+").enabled(zoom < 18);
  ui.findButtonByText("-").enabled(zoom > 12);
  drawMap();
  ui.update();
}

void onAddrBtnPressed(Button& btnAddr) {
  btnAddr._pressed = isKeyboardActive;
  btnAddr.updateAfterMs = 0;
  toggleKeyboard();
}

void onRouteBtnPressed(Button& _) {
  ui.update();
  router.search(myLoc.lat ? myLoc : centerLoc, targetLoc, router.BIKE);
  drawMap();
  ui.update();
}

void onAddrType(Button& btn) {
  Input& inp = ui.findInputById('a');
  if (btn.text == "<") {
    inp.removeChar();
    ui.drawInput(inp);
  } else if (btn.text == " ") {
    inp.addChar('+');
    ui.drawInput(inp);
  } else if (btn.text == ">") {
    ui.update();
    if (address.search(inp.text, centerLoc)) showAddresses();
    toggleKeyboard();
  } else {
    inp.addChar(btn.text[0]);
    ui.drawInput(inp);
  }
}

void onClick(const Pos& p) {
  if (!ui.processPress(p.x, p.y)) {
    targetLoc = coord::pointToLocation(p, centerLoc, zoom);
    ui.findButtonByText("R").enabled(true);
    drawMap();
    ui.update();
  }
}

void onDrag(const Pos& p) {
  Point start = coord::locationToPixels(centerLoc, zoom);
  Point end = {start.x - p.dx, start.y - p.dy};
  centerLoc = coord::pixelsToLocation(end, zoom);
  drawMap();
  ui.updateAfter(ANIM_MS * 2);
}

void createKeyboard() {
  char _keys[] = KEYBOARD;
  int idx = 0;
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 10; x++) {
      if (idx == 40) break;
      ui.addButton(_keys[idx++], x * (BUTTON_W + BUTTON_SPACING) + KEYBOARD_X, y * (BUTTON_H + BUTTON_SPACING) + KEYBOARD_Y)
        .type('k')
        .visible(false)
        .onPress(onAddrType);
    }
  }
  for (int i = 0; i < ADDR_SEARCH_LIMIT; i++) // addresses
    ui.addButton("", 0, i * 51,SCREEN_WIDTH, 50, i)
      .visible(false)
      .type('a')
      .onPress(onAddresPressed);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  int waitCount = 0;
  while (!Serial && waitCount < 300) { // 3 seconds
    delay(10);
    waitCount++;
  }

  print("--------------- started ---------------");

  spiDisplay.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
  TFT.init();
  TFT.setRotation(2);
  TFT.fillScreen(TFT_BLACK);
  TFT.setTextColor(TFT_WHITE);
  TFT.setFreeFont(&FreeMono9pt7b);
  TFT.setCursor(0, 20);

  TFT.print("Init UI");
  ui.init(TFT);
  ui.addButton('+', 10, 10).enabled(zoom < 18).onPress(onZoomBtnPressed);
  ui.addButton('-', 10, 10 + 45).enabled(zoom > 12).onPress(onZoomBtnPressed);
  ui.addButton('L', 10, 10 + 45 * 2).enabled(false);
  ui.addButton('A', 10, 10 + 45 * 3).onPress(onAddrBtnPressed);
  ui.addButton('R', 10, 10 + 45 * 4).enabled(false).onPress(onRouteBtnPressed);
  ui.addInput(0, KEYBOARD_Y - 40,SCREEN_WIDTH,BUTTON_H, 'a').visible(false);
  createKeyboard();

  TFT.print("Init card reader");

  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  int frq = 80000000;
  while (!SD.begin(SD_CS, spiSD, frq) && frq > 1000000) {
    frq -= 1000000;
    delay(100);
    TFT.print(".");
  }
  TFT.println("");
  TFT.print("SD speed:");
  print("SD speed:", frq);
  TFT.println(frq);
  TFT.println("Init SD card");

  while (SD.cardType() == CARD_NONE) {
    TFT.print(".");
    print("No SD card attached");
    delay(500);
  }
  TFT.println("SD card is OK");

  TFT.println("Init touch");
  Wire.begin(I2C_SDA, I2C_SCL, 0);
  delay(100);
  if (!touch.init()) {
    TFT.println("Init touch fail");
  } else {
    touch.onClick(onClick);
    touch.onDrag(onDrag);
    TFT.println("Touch is OK");
  }

  TFT.println("Init GPS");
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  TFT.println("Init compass");
  compass.init();
  compass.enableDefault();
  compass.m_min = (LSM303::vector<int16_t>){-686, -545, -4};
  compass.m_max = (LSM303::vector<int16_t>){+331, +353, +4};

  TFT.println("Init cache");
  initializeCache();

  TFT.println("Init WIFI");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    TFT.println("Connecting to WiFi...");
  }
  TFT.println("Connected to WiFi");

  TFT.println("Init done!");

  TFT.setTextColor(TFT_BLACK);
  drawMap();
  ui.update();

  now = millis();
  compassUpdateAfterMs = now + COMPASS_UPDATE_PERIOD;
  gpsUpdateAfterMs = now + GPS_UPDATE_PERIOD;
}

void loop() {
  now = millis();
  touch.update();

  if (now > compassUpdateAfterMs) {
    compass.read();
    new_angle = compass.heading();
    if (abs(new_angle - angle) > COMPASS_ANGLE_STEP) {
      angle = new_angle;
      drawMyMarker();
    }
    compassUpdateAfterMs = now + COMPASS_UPDATE_PERIOD;
  }

  if (now > gpsUpdateAfterMs) {
    while (gpsSerial.available() > 0) {
      gps.encode(gpsSerial.read());
    }
    if (gps.location.isUpdated()) {
      float gpsLat = gps.location.lat();
      float gpsLon = gps.location.lng();
      if (std::abs(myLoc.lat - gpsLat) > MIN_COORD_CHANGE || (std::abs(myLoc.lon - gpsLon) > MIN_COORD_CHANGE)) {
        myLoc.lat = gpsLat;
        myLoc.lon = gpsLon;
        Button locBtn = ui.findButtonByText("L");
        if (!locBtn.enabled()) { // todo: support the lost location case
          locBtn.enabled(true);
          ui.update();
        }
      }
    }
    gpsUpdateAfterMs = now + GPS_UPDATE_PERIOD;
  }

  if (ui.updateAfterMs != 0 && now > ui.updateAfterMs) ui.update();
}
