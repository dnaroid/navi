#include <PathFinder.h>
#include <SD.h>
#include <secrets.h>
#include <SPI.h>
#include <Wire.h>
#include "WiFi.h"
#include "LSM303.h"
#include "lvgl.h"
#include "MapUI.h"
#include "Touch.h"
#include "Display.h"
#include "BootManager.h"
#include "TinyGPSPlus.h"
#include "../lv_conf.h"

// global
float compass_angle;
Location my_location = {0, 0};

TinyGPSPlus gps;
LSM303 compass;
HardwareSerial gpsSerial(1);

static BootState state;
static Mode mode = ModeMap;
static PathFinder pf;
auto spiSD = SPIClass(FSPI);


void displayGPSInfo() {
  Serial.print(F("Location: "));
  if (gps.location.isValid()) {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  } else {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid()) {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  } else {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid()) {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  } else {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}

int gpsSkips = 0;
int gpsInfoSkips = 0;

void updateCompassAndGPS(void* pvParameters) {
  while (true) {
#ifndef DISABLE_COMPASS
    compass.read();
    compass_angle = compass.heading();
#endif
#ifndef DISABLE_GPS
    if (gpsSkips++ < GPS_UPD_SKIPS) continue;
    while (gpsSerial.available() > 0) {
      gps.encode(gpsSerial.read());
      if (gpsInfoSkips++ > 100) {
        displayGPSInfo();
        gpsInfoSkips = 0;
      }
    }
    if (gps.location.isUpdated()) {
      float gpsLat = gps.location.lat();
      float gpsLon = gps.location.lng();
      my_location.lat = gpsLat;
      my_location.lon = gpsLon;
    }
    gpsSkips = 0;
#endif
    delay(COMPASS_UPD_PERIOD);
  }
}

void setup() {
  START_SERIAL

  btStop(); // Bluetooth OFF

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
#ifndef DISABLE_GPS
    LOGI("Init GPS");
    gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
    LOG(" ok");
#endif

    Map_init(state);

    xTaskCreatePinnedToCore(updateCompassAndGPS, "UpdateTask", 4096, NULL, 1, NULL, 1);
    break;

  case ModeRoute:
    sqlite3_initialize();
    pf.init();
    pf.findPath(state.start, state.end);
    pf.calculateMapCenterAndZoom();
    writeBootState({CURRENT_BM_VER, ModeMap, pf.pathCenter, pf.zoom, state.start, state.end, pf.path, pf.distance});
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

#ifndef DISABLE_SERVER
  ServerLoop();
#endif
}
