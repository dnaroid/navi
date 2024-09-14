#include <PathFinder.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include "lvgl.h"
#include "MapUI.h"
#include "Touch.h"
#include "Display.h"
#include "BootManager.h"
#include "TinyGPSPlus.h"
#include "../lv_conf.h"

#ifdef MIRROR
#include "Mirror.h"
SemaphoreHandle_t xGuiSemaphore;
#endif

#ifdef MINI_TFT
#include <DFRobot_QMC5883.h>
DFRobot_QMC5883 compass(&Wire, QMC5883_ADDRESS);
#else
#include "LSM303.h"
#include "compass_calibrate.h"
static LSM303 compass;
#endif


// global
float compass_angle;
Location my_gps_location = {0, 0};

static TinyGPSPlus gps;
static HardwareSerial gpsSerial(1);
#ifdef MINI_TFT
static auto spiSD = SPIClass(VSPI);
#else
static auto spiShared = SPIClass(HSPI);
#endif
static BootState state;
static Mode mode = ModeMap;
static PathFinder pf;

static int gpsSkips = 0;
static int compassSkips = 0;
static bool isLowFrequency = false;

void updateCompassAndGpsTask(void* pvParameters) {
  while (true) {
    if (compassSkips++ > COMPASS_UPD_SKIPS) {
#ifdef MINI_TFT
      sVector_t mag = compass.readRaw();
      compass.getHeadingDegrees();
      compass_angle = mag.HeadingDegress;
#else
      compass.read();
      compass_angle = compass.heading();
#endif
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
      if (!gps.location.isValid()) {
        my_gps_location.lat = 0;
        my_gps_location.lon = 0;
      }
      gpsSkips = 0;
    }
#ifdef MIRROR
    Mirror_loop();
    // delay(10);
#else
    delay(100);
#endif
  }
}

void setup() {
  START_SERIAL

  LOGI("Init Card reader");
#ifdef MINI_TFT
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  while (!SD.begin(SD_CS, spiSD, 80000000, "/sd", 10)) {
#else
  spiShared.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  while (!SD.begin(SD_CS, spiShared, 80000000, "/sd", 10)) {
#endif
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
#ifdef MIRROR
    xGuiSemaphore = xSemaphoreCreateMutex();
#endif

    Display_init();

    Wire.begin(I2C_SDA, I2C_SCL);

    LOGI("Init Compass");
#ifdef MINI_TFT
    while (!compass.begin()) {
      Serial.println("Could not find a valid QMC5883 sensor, check wiring!");
      delay(1000);
    }

  /**
  * @brief  Set declination angle on your location and fix heading
  * @n      You can find your declination on: http://magnetic-declination.com/
  * @n      (+) Positive or (-) for negative
  * @n      For Bytom / Poland declination angle is 4'26E (positive)
  * @n      Formula: (deg + (min / 60.0)) / (180 / PI);
  */
    compass.setDeclinationAngle((6.0 + (45.0 / 60.0)) / (180 / PI));

    compass.setRange(QMC5883_RANGE_2GA);
    compass.setMeasurementMode(QMC5883_CONTINOUS);
    compass.setDataRate(QMC5883_DATARATE_50HZ);
    compass.setSamples(QMC5883_SAMPLES_8);

#else
    compass.init();
    compass.enableDefault();
    if (!loadCalibrationData(compass)) {
      TFT_eSPI tft;
      tft.init();
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE);
      tft.setCursor(0, 20);
      tft.println("Compass is not calibrated!");
      tft.println("Rotate the devices along all axes for 10 seconds...");
      calibrateCompass(compass);
      esp_restart();
    }
#endif
    LOG(" ok");

    LOGI("Init GPS");
    gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
    LOG(" ok");

    Touch_init();

  // Map_init(state);

    Mirror_init();
    Mirror_start();

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

void setHighFrequency() {
  setCpuFrequencyMhz(240);
  isLowFrequency = false;
}

void setLowFrequency() {
  setCpuFrequencyMhz(80);
  isLowFrequency = true;
}


void loop() {
#ifdef MIRROR
  // if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE) {
  //   lv_timer_handler();
  //   xSemaphoreGive(xGuiSemaphore);
  // }
#else
  lv_timer_handler();
#ifndef MINI_TFT
  if (lv_display_get_inactive_time(NULL) < IDLE_TIME_MS) {
    if (isLowFrequency) setHighFrequency();
  } else {
    if (!isLowFrequency) setLowFrequency();
  }
#endif
#endif
}
