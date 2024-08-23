#include <PathFinder.h>
#include <SD.h>
#include <SDCard.h>
#include <SPI.h>
#include "WiFi.h"
#include "LSM303.h"
#include "sqlite3.h"
#include "lvgl.h"
#include "../lv_conf.h"
#include "MapUI.h"
#include "Touch.h"
#include "Display.h"
#include "BootDispatcher.h"
// #include "rd.h"

static BootState state;
static Mode mode = ModeMap;
static PathFinder pf;
auto spiSD = SPIClass(FSPI);

// void drawRoute() {
//   const auto route = pathFinder.path;
//   if (route.empty()) { return; }
//   Point p1 = coord::locationToScreen(route[0], centerLoc, zoom);
//   for (size_t i = 1; i < route.size(); i++) {
//     const Point p2 = coord::locationToScreen(route[i], centerLoc, zoom);
//     TFT.drawLine(p1.x, p1.y, p2.x, p2.y,TFT_BLUE);
//     TFT.drawLine(p1.x, p1.y + 1, p2.x, p2.y + 1,TFT_BLUE);
//     TFT.drawLine(p1.x, p1.y - 1, p2.x, p2.y - 1,TFT_BLUE);
//     TFT.drawLine(p1.x + 1, p1.y, p2.x + 1, p2.y,TFT_BLUE);
//     TFT.drawLine(p1.x - 1, p1.y, p2.x - 1, p2.y,TFT_BLUE);
//     p1 = p2;
//   }
// }

// void searchAddress(const String& text) {
//   foundAddrs.clear();
//   String modifiedText = text;
//   modifiedText.replace(' ', '%');
//   const String query = "SELECT str, num, lon, lat, details FROM addr WHERE alias LIKE '%"
//     + modifiedText
//     + "%' ORDER BY CAST(num AS INTEGER) ASC LIMIT "
//     + ADDR_SEARCH_LIMIT;
//   const char* queryCStr = query.c_str();
//   sqlite3_exec(addrDb, queryCStr,
//                [](void* data, int argc, char** argv, char** azColName) -> int {
//                  const String name = String(argv[0]) + " " + String(argv[1]) + " " + String(argv[4]);
//                  const float lon = atof(argv[2]);
//                  const float lat = atof(argv[3]);
//                  foundAddrs.push_back(Address{name, {lon, lat}});
//                  return 0;
//                }, (void*)dbData, &zErrMsg);
//   showAddresses();
// }


void setup() {
  START_SERIAL

  btStop(); // Bluetooth OFF


#ifndef DISABLE_GPS
  gpsUpdateAfterMs = now + GPS_UPDATE_PERIOD;
#endif

#ifndef DISABLE_COMPASS
  LOGI("Init Compass");
  compass.init();
  compass.enableDefault();
  compass.m_min = (LSM303::vector<int16_t>){-686, -545, -4};
  compass.m_max = (LSM303::vector<int16_t>){+331, +353, +4};
  LOG(" ok");
  compassUpdateAfterMs = now + COMPASS_UPDATE_PERIOD;
#endif

#ifndef DISABLE_SERVER
  // WiFi.persistent(false);
  ServerSetup();
#endif

  LOGI("Init Card reader");
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  while (!SD.begin(SD_CS, spiSD, 80000000)) {
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

  state = readBootState();
  mode = state.mode;

  switch (mode) {
  case ModeMap:
    // SDCard_init();
    Display_init();
    Touch_init();
    Map_init(state.center, state.zoom, state.target, state.distance, state.route);
    break;

  case ModeRoute:
    sqlite3_initialize();
    pf.init();
    pf.findPath(state.center, state.target);
    writeBootState({ModeMap, state.center, state.zoom, state.target, pf.path, pf.distance});
    esp_restart();
    break;

  case ModeMirror:
    break;
  }

  LOG("---------------- Init done ----------------");
}

void loop() {
  if (mode == ModeMap) {
    lv_timer_handler();
    lv_tick_inc(10);
    delay(10);
  } else if (mode == ModeRoute) {
  } else if (mode == ModeMirror) {
  }

  return;


#ifndef DISABLE_COMPASS
  if (now > compassUpdateAfterMs) {
    compass.read();
    new_angle = compass.heading();
    if (abs(new_angle - angle) > COMPASS_ANGLE_STEP) {
      angle = new_angle;
      drawMyMarker();
    }
    compassUpdateAfterMs = now + COMPASS_UPDATE_PERIOD;
  }
#endif

#ifndef DISABLE_GPS
  bool gps = false;

  if (now > gpsUpdateAfterMs) {
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
    // a9g.loop(gps);

    gpsUpdateAfterMs = now + GPS_UPDATE_PERIOD;
  }
#endif

#ifndef DISABLE_UI
  if (ui.updateAfterMs != 0 && now > ui.updateAfterMs) ui.update();
#endif

#ifndef DISABLE_SERVER
  ServerLoop();
#endif
}
