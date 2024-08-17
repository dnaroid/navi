#include <esp_wifi.h>

#include "globals.h"
#include <PNGdec.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

#include <TFT_eSPI.h>
#include <WiFi.h>

#include "coord.h"

#include "PngTile.h"

#include "LSM303.h"
LSM303 compass;

#include "UI.h"
UI ui;

#include "Touch.h"
Touch touch;

#include <sqlite3.h>
sqlite3* addrDb;

#include "PathFinder.h"
PathFinder pathFinder;

#include "Server.h"

// TinyGPSPlus gps;
// HardwareSerial gpsSerial(1);
// HardwareSerial atSerial(2);

// TinyGsm modem(atSerial);
// TinyGsmClient client(modem);
// Router router;
// auto a9g = A9G(gpsSerial);

char* zErrMsg = 0;
int rc;
const char* dbData = "Callback function called";
std::vector<Address> foundAddrs; //todo make class

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
  constexpr int radius = 10;
  constexpr int dy = -(radius * 1.5);
  constexpr int color = TFT_BLUE;
  Point p = coord::locationToScreen(targetLoc, centerLoc, zoom);
  TFT.fillCircle(p.x, p.y + dy, radius, color);
  TFT.fillTriangle(p.x - radius, p.y + dy, p.x + radius, p.y + dy, p.x, p.y, color);
  TFT.fillCircle(p.x, p.y + dy, radius / 3, TFT_WHITE);
}

void drawRoute() {
  const auto route = pathFinder.path;
  if (route.empty()) { return; }
  Point p1 = coord::locationToScreen(route[0], centerLoc, zoom);
  for (size_t i = 1; i < route.size(); i++) {
    const Point p2 = coord::locationToScreen(route[i], centerLoc, zoom);
    TFT.drawLine(p1.x, p1.y, p2.x, p2.y,TFT_BLUE);
    TFT.drawLine(p1.x, p1.y + 1, p2.x, p2.y + 1,TFT_BLUE);
    TFT.drawLine(p1.x, p1.y - 1, p2.x, p2.y - 1,TFT_BLUE);
    TFT.drawLine(p1.x + 1, p1.y, p2.x + 1, p2.y,TFT_BLUE);
    TFT.drawLine(p1.x - 1, p1.y, p2.x - 1, p2.y,TFT_BLUE);
    p1 = p2;
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
  for (const auto res : foundAddrs) {
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
  centerLoc = foundAddrs[btn.id].location;
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
  pathFinder.findPath(myLoc.lat ? myLoc : centerLoc, targetLoc);
  drawMap();
  ui.update();
}

void searchAddress(const String& text) {
  foundAddrs.clear();
  String modifiedText = text;
  modifiedText.replace(' ', '%');
  const String query = "SELECT str, num, lon, lat, details FROM addr WHERE alias LIKE '%"
    + modifiedText
    + "%' ORDER BY CAST(num AS INTEGER) ASC LIMIT "
    + ADDR_SEARCH_LIMIT;
  const char* queryCStr = query.c_str();
  sqlite3_exec(addrDb, queryCStr,
               [](void* data, int argc, char** argv, char** azColName) -> int {
                 const String name = String(argv[0]) + " " + String(argv[1]) + " " + String(argv[4]);
                 const float lon = atof(argv[2]);
                 const float lat = atof(argv[3]);
                 foundAddrs.push_back(Address{name, {lon, lat}});
                 return 0;
               }, (void*)dbData, &zErrMsg);
  showAddresses();
}

void onAddrType(Button& btn) {
  Input& inp = ui.findInputById('a');
  if (btn.text == "<") {
    inp.removeChar();
    ui.drawInput(inp);
  } else if (btn.text == ">") {
    ui.update();
    searchAddress(inp.text);
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
  constexpr int addrH = SCREEN_HEIGHT / ADDR_SEARCH_LIMIT;
  for (int i = 0; i < ADDR_SEARCH_LIMIT; i++) // addresses
    ui.addButton("", 0, i * addrH,SCREEN_WIDTH, addrH, i)
      .visible(false)
      .type('a')
      .onPress(onAddresPressed);
}

int dbOpen(const char* filename, sqlite3** db) {
  int rc = sqlite3_open(filename, db);
  if (rc) {
    Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
    return rc;
  }
  Serial.printf("Opened database successfully\n");
  return rc;
}

void setupUI() {
  LOGI("Init cache ");
  initializeCache();
  LOG("ok");
  LOGI("Init TFT ");
  TFT.init();
  // TFT.setAttribute(UTF8_SWITCH, 1);
  TFT.setRotation(2);
  TFT.fillScreen(TFT_BLACK);
  TFT.setTextColor(TFT_WHITE);
  TFT.setFreeFont(&FreeMono9pt7b);
  TFT.setCursor(0, 20);

  ui.init(TFT);
  ui.addButton('+', 10, 10).enabled(zoom < 18).onPress(onZoomBtnPressed);
  ui.addButton('-', 10, 10 + 45).enabled(zoom > 12).onPress(onZoomBtnPressed);
  ui.addButton('L', 10, 10 + 45 * 2).enabled(false);
  ui.addButton('A', 10, 10 + 45 * 3).onPress(onAddrBtnPressed);
  ui.addButton('R', 10, 10 + 45 * 4).enabled(false).onPress(onRouteBtnPressed);
  ui.addInput(0, KEYBOARD_Y - 40,SCREEN_WIDTH,BUTTON_H, 'a').visible(false);
  createKeyboard();
  LOG("ok");
  LOGI("Init Touch ");
  Wire.begin(I2C_SDA, I2C_SCL, 0);
  delay(100);
  if (!touch.init()) {
    LOG("fail");
  } else {
    touch.onClick(onClick);
    touch.onDrag(onDrag);
    LOG("ok");
  }
  isReadyUI = true;
}

void setupSD() {
  LOGI("Init card reader");
  auto spiSD = SPIClass(FSPI);
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  int frq = 80000000;
  while (!SD.begin(SD_CS, spiSD, frq) && frq > 1000000) {
    // frq -= 1000000;
    delay(100);
    LOGI(".");
  }
  LOG(" ok");
  LOGI("Init SD card");
  while (SD.cardType() == CARD_NONE) {
    LOGI(".");
    delay(100);
  }
  LOG(" ok");
  isReadySD = true;
}

void setupDB() {
  if (!isReadySD) {
    LOG("SD is not ready. Skip DB init");
    return;
  }
  LOGI("Init DB ");
  sqlite3_initialize();
  constexpr int flags = SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX;
  rc = sqlite3_open_v2("/sd/addr.db", &addrDb, flags, nullptr);
  if (rc != SQLITE_OK) {
    sqlite3_close(addrDb);
    LOG("fail");
  } else {
    pathFinder.init();
    LOG("ok");
    isReadyDB = true;
  }
}

void setupCompass() {
  LOGI("Init Compass");
  compass.init();
  compass.enableDefault();
  compass.m_min = (LSM303::vector<int16_t>){-686, -545, -4};
  compass.m_max = (LSM303::vector<int16_t>){+331, +353, +4};
  LOG(" ok");
  isReadyCompass = true;
}


void setup() {
  int waitCount = 0;
  while (!Serial && waitCount < 50) { // 5 seconds
    Serial.begin(115200);
    delay(100);
    waitCount++;
  }
  Serial.println("--------------- started ---------------");

  // btStop(); // Bluetooth OFF

  setupSD();
  // setupDB();
  // setupCompass();
  setupUI();

  ServerSetup();

  LOG("----------------Init done----------------");

  compassUpdateAfterMs = now + COMPASS_UPDATE_PERIOD;
  gpsUpdateAfterMs = now + GPS_UPDATE_PERIOD;

  TFT.setTextColor(TFT_BLACK);
  // drawMap();
  ui.update();
}

void loop() {
  now = millis();
  touch.update();
  bool gps = false;

  if (isReadyCompass && now > compassUpdateAfterMs) {
    compass.read();
    new_angle = compass.heading();
    if (abs(new_angle - angle) > COMPASS_ANGLE_STEP) {
      angle = new_angle;
      drawMyMarker();
    }
    compassUpdateAfterMs = now + COMPASS_UPDATE_PERIOD;
  }

  if (isReadyGPS && now > gpsUpdateAfterMs) {
    gps = true;
    // while (gpsSerial.available() > 0) {
    //   gps.encode(gpsSerial.read());
    // }
    // if (gps.location.isUpdated()) {
    //   float gpsLat = gps.location.lat();
    //   float gpsLon = gps.location.lng();
    //   if (std::abs(myLoc.lat - gpsLat) > MIN_COORD_CHANGE || (std::abs(myLoc.lon - gpsLon) > MIN_COORD_CHANGE)) {
    //     myLoc.lat = gpsLat;
    //     myLoc.lon = gpsLon;
    //     Button locBtn = ui.findButtonByText("L");
    //     if (!locBtn.enabled()) { // todo: support the lost location case
    //       locBtn.enabled(true);
    //       ui.update();
    //     }
    //   }
    // }
    gpsUpdateAfterMs = now + GPS_UPDATE_PERIOD;
  }

  if (ui.updateAfterMs != 0 && now > ui.updateAfterMs) ui.update();
  // a9g.loop(gps);
}
