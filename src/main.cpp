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

#include "../lv_conf.h"

// global
float compass_angle;
Location my_gps_location = {0, 0};

TinyGPSPlus gps;
LSM303 compass;
HardwareSerial gpsSerial(1);

SemaphoreHandle_t xGuiSemaphore;

static BootState state;
static Mode mode = ModeMap;
static PathFinder pf;

auto spiShared = SPIClass(HSPI);
int gpsSkips = 0;
int compassSkips = 0;

void updateCompassAndGpsTask(void* pvParameters) {
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
        my_gps_location.lat = gpsLat;
        my_gps_location.lon = gpsLon;
      }
      gpsSkips = 0;
    }
    Mirror_loop();
    delay(10);
  }
}

void setup() {
  START_SERIAL

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

    xGuiSemaphore = xSemaphoreCreateMutex();

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

    Map_init(state);

    xTaskCreatePinnedToCore(updateCompassAndGpsTask, "UpdateTask", 4096, NULL, 1, NULL, 1);

    break;

  case ModeRoute:
    sqlite3_initialize();
    pf.init(state.transport);
    pf.findPath(state.start, state.end);
    pf.calculateMapCenterAndZoom();
    writeBootState({CURRENT_BM_VER, ModeMap, state.transport, pf.pathCenter, pf.zoom, state.start, state.end, pf.distance, pf.path,});
    esp_restart();
  }

  LOG("---------------- Init done ----------------");
}

void loop() {
  if (mode == ModeMap) {
    if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE) {
      lv_timer_handler();
      xSemaphoreGive(xGuiSemaphore);
    }
  }
}
