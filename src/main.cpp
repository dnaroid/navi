#include <PathFinder.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "WiFi.h"
#include "LSM303.h"
#include "lvgl.h"
#include "MapUI.h"
#include "Touch.h"
#include "Display.h"
#include "BootManager.h"
#include "../lv_conf.h"

static BootState state;
static Mode mode = ModeMap;
static PathFinder pf;
auto spiSD = SPIClass(FSPI);
LSM303 compass;

float compass_angle;

void updateCompassAndGPS(void* pvParameters) {
  while (true) {
#ifndef DISABLE_COMPASS
    compass.read();
    compass_angle = compass.heading();
  }
#endif
  delay(COMPASS_UPD_PERIOD);
}

void setup() {
  START_SERIAL

  btStop(); // Bluetooth OFF

#ifndef DISABLE_GPS
  gpsUpdateAfterMs = now + GPS_UPDATE_PERIOD;
#endif

#ifndef DISABLE_SERVER
  WiFi.persistent(false);
  ServerSetup();
#endif

  LOGI("Init Card reader");
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  while (!SD.begin(SD_CS, spiSD, 70000000, "/sd", 10)) {
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
    Wire.begin(I2C_SDA, I2C_SCL, 0);
    delay(100);

    Display_init();

    Touch_init();

#ifndef DISABLE_COMPASS
    LOGI("Init Compass");
    compass.init();
    compass.enableDefault();
    compass.m_min = (LSM303::vector<int16_t>){-686, -545, -4};
    compass.m_max = (LSM303::vector<int16_t>){+331, +353, +4};
    LOG(" ok");
#endif

    Map_init(state);

    xTaskCreatePinnedToCore(updateCompassAndGPS, "UpdateTask", 4096, NULL, 1, NULL, 1);
    break;

  case ModeRoute:
    sqlite3_initialize();
    pf.init();
    pf.findPath(state.start, state.end);
    writeBootState({CURRENT_BM_VER, ModeMap, state.center, state.zoom, state.start, state.end, pf.path, pf.distance});
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

#ifndef DISABLE_SERVER
  ServerLoop();
#endif
}
