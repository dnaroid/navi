#include <esp_bt.h>
#include <esp_bt_main.h>
#include <PathFinder.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include "LSM303.h"
#include "lvgl.h"
#include "MapUI.h"
#include "Touch.h"
#include "Display.h"
#include "Mirror.h"
#include "BootManager.h"
#include "TinyGPSPlus.h"
#include <TJpg_Decoder.h>

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

auto spiShared = SPIClass(HSPI);
int gpsSkips = 0;
int compassSkips = 0;

void updateCompassAndGPS(void* pvParameters) {
  while (true) {
    if (compassSkips++ > COMPASS_UPD_SKIPS) {
      compass.read();
      compass_angle = compass.heading();
      compassSkips = 0;
    }

    if (gpsSkips++ > GPS_UPD_SKIPS) {
      while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
      }
      if (gps.location.isUpdated()) {
        float gpsLat = gps.location.lat();
        float gpsLon = gps.location.lng();
        my_location.lat = gpsLat;
        my_location.lon = gpsLon;
      }
      gpsSkips = 0;
    }

    Mirror_loop();
  }
}

void disableBluetooth() {
  if (esp_bluedroid_disable() == ESP_OK) {
    Serial.println("Bluetooth stack disabled");
  } else {
    Serial.println("Failed to disable Bluetooth stack");
  }

  if (esp_bt_controller_disable() == ESP_OK) {
    Serial.println("Bluetooth controller disabled");
  } else {
    Serial.println("Failed to disable Bluetooth controller");
  }
}


void setup() {
  START_SERIAL

  disableBluetooth();

  LOGI("Init Card reader");
  spiShared.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  while (!SD.begin(SD_CS, spiShared, 80000000, "/sd", 10)) {
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

    LOGI("Init Compass");
    compass.init();
    compass.enableDefault();
    compass.m_min = (LSM303::vector<int16_t>){-686, -545, -4};
    compass.m_max = (LSM303::vector<int16_t>){+331, +353, +4};
    LOG(" ok");

    LOGI("Init GPS");
    gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
    LOG(" ok");

  // Map_init(state);

    LOGI("Init Cam");
    Mirror_init();
    Mirror_start();
    LOG(" ok");

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
    // auto tft = TFT_eSPI();
    // tft.begin();
    // tft.initDMA();
    // tft.invertDisplay(true);

    break;
  }

  LOG("---------------- Init done ----------------");
}

void loop() {
  if (mode == ModeMap) {
    lv_timer_handler();
  } else if (mode == ModeRoute) {
  } else if (mode == ModeMirror) {
  }
}
