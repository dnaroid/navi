#ifndef GLOBALS_H
#define GLOBALS_H

#include "Arduino.h"

struct Location {
  float lon;
  float lat;
};

extern float compass_angle;
extern Location my_location;

#define INIT_ZOOM 16

/// --- features disabling
#define DISABLE_SERVER
// #define DISABLE_COMPASS
// #define DISABLE_GPS

/// --- pins
#define SD_CS 7
#define SD_MOSI 17
#define SD_SCK 16
#define SD_MISO 15

#define I2C_SDA 18
#define I2C_SCL 9

#define GPS_RX 47
#define GPS_TX 48

/// --- screen
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480
#define TILE_SIZE 256
#define ZOOM_MIN 12
#define ZOOM_MAX 18

/// --- compass
#define COMPASS_ANGLE_STEP 10
#define COMPASS_ANGLE_CORRECTION 0
#define COMPASS_MAGNETIC_DECLINATION_D 6
#define COMPASS_MAGNETIC_DECLINATION_M 43

/// --- timing
#define COMPASS_UPD_PERIOD 100
#define GPS_UPD_SKIPS 20

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

