#ifndef GLOBALS_H
#define GLOBALS_H

#include "Arduino.h"

#define LV_LVGL_H_INCLUDE_SIMPLE

#ifdef MIRROR
#ifdef MIRROR_UART
// #define MIRROR_UART_BAUD 115200
#define MIRROR_UART_BAUD 4000000
#define MIRROR_RX 1
#define MIRROR_TX 2
#else
#define WIFI_SSID "ESP32-SCOOTER"
#define WIFI_PASSWORD "UoAcYyo5FErnjXk"
#define IMAGE_CAPTURE_URL "http://192.168.4.1/jpg"
#endif
#define COMPASS_UPD_SKIPS 10
#define GPS_UPD_SKIPS 50
#else
#define COMPASS_UPD_SKIPS 1
#define GPS_UPD_SKIPS 5
#endif

#define INIT_LAT 54.3520
#define INIT_LON 18.6466

#define ZOOM_DEFAULT 16
#define ZOOM_TRIP 17
#define ZOOM_MIN 12
#define ZOOM_MAX 18

struct Location {
  float lon;
  float lat;
};

extern float compass_angle;
extern Location my_gps_location;

/// --- pins
#ifdef MINI_TFT
#define SD_CS   38
#define SD_MOSI 39
#define SD_SCK  40
#define SD_MISO 41

#define I2C_SDA  6
#define I2C_SCL  7
#else
#define SD_CS    7
#define SD_MOSI 11
#define SD_SCK  12
#define SD_MISO 13

#define I2C_SDA 18
#define I2C_SCL  9
#endif

#define GPS_RX 47
#define GPS_TX 48

/// --- screen
#ifdef MINI_TFT
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 280
#define BUTTON_W 50
#define BUTTON_H 35
#define BUTTONS_R_OFFSET 15
#else
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480
#define BUTTON_W 40
#define BUTTON_H 26
#define BUTTONS_R_OFFSET 5
#endif
#define SCREEN_CENTER_X (SCREEN_WIDTH/2)
#define SCREEN_CENTER_Y (SCREEN_HEIGHT/2)
#define TILE_SIZE 256

#define IDLE_TIME_MS 10000

/// --- compass
#define COMPASS_ANGLE_CORRECTION (-90)


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

#ifndef DEBUG
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

