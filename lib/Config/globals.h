#ifndef GLOBALS_H
#define GLOBALS_H

#include "Arduino.h"

#define LV_LVGL_H_INCLUDE_SIMPLE

#define WIFI_SSID "ESP32-SCOOTER"
#define WIFI_PASSWORD "UoAcYyo5FErnjXk"

#define INIT_LAT 54.3520
#define INIT_LON 18.6466

struct Location {
  float lon;
  float lat;
};

extern float compass_angle;
extern Location my_gps_location;

#define ZOOM_DEFAULT 16

/// --- pins
#define SD_CS    7
#define SD_MOSI 11
#define SD_SCK  12
#define SD_MISO 13

#define I2C_SDA 18
#define I2C_SCL  9

#define GPS_RX 47
#define GPS_TX 48

/// --- screen
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480
#define SCREEN_CENTER_X (SCREEN_WIDTH/2)
#define SCREEN_CENTER_Y (SCREEN_HEIGHT/2)
#define TILE_SIZE 256
#define ZOOM_MIN 12
#define ZOOM_MAX 18

/// --- compass
#define COMPASS_ANGLE_CORRECTION (90)

/// --- timing
#define COMPASS_UPD_SKIPS 2
#define GPS_UPD_SKIPS 100

/// remote
#define IMAGE_CAPTURE_URL "http://192.168.4.1/jpg"

inline void print() {
}

template <typename T, typename... Args>
void print(T first, Args... args) {
  Serial.print(first);
  Serial.print(' ');
  print(args...);
}

template <typename... Args>
void println(Args... args) {
  print(args...);
  Serial.println();
}

#ifdef RELEASE
#define START_SERIAL
#define LOG(...)
#define LOGI(...)
#define LOGF(...)
#else
#define LOG(...) println(__VA_ARGS__)
#define LOGI(...) print(__VA_ARGS__)
#define LOGF(...) Serial.printf(__VA_ARGS__)
#define START_SERIAL int waitCount = 0;\
while (!Serial.available() && waitCount++ < 50) { \
  Serial.begin(115200);\
  delay(100);\
}\
Serial.println("--------------- started ---------------");
#endif

#define delay(ms) vTaskDelay(ms / portTICK_PERIOD_MS)

#endif //GLOBALS_H

