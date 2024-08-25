#ifndef GLOBALS_H
#define GLOBALS_H

#include "Arduino.h"

#define INIT_ZOOM 16

/// --- features disabling
#define DISABLE_SERVER
#define DISABLE_COMPASS
#define DISABLE_GPS

#define TINY_GSM_MODEM_SIM800

/// --- pins
#define SD_CS 7
#define SD_MOSI 17
#define SD_SCK 16
#define SD_MISO 15

#define I2C_SDA 18
#define I2C_SCL 9

#define GPS_RX 47
#define GPS_TX 48

// #define A9G_PON 4
// #define A9G_LOWP 5
// #define A9G_RST 6

/// --- screen
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480


/// --- compass
#define COMPASS_ANGLE_STEP 10
#define COMPASS_ANGLE_CORRECTION 0
#define COMPASS_MAGNETIC_DECLINATION_D 6
#define COMPASS_MAGNETIC_DECLINATION_M 43

/// --- timing
#define COMPASS_UPDATE_PERIOD 1000
#define GPS_UPDATE_PERIOD 10000

/// --- GPS
#define MIN_COORD_CHANGE 0.0001

/// remote
#define ADDR_SEARCH_LIMIT 5
#define IMAGE_CAPTURE_URL "http://192.168.4.1/jpg"

#define TINY_GSM_RX_BUFFER 650

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG Serial
// #define LOGGING  // <- Logging is for the HTTP library

// Range to attempt to autobaud
// NOTE:  DO NOT AUTOBAUD in production code.  Once you've established
// communication, set a fixed baud rate using modem.setBaud(#).
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

#define LV_LVGL_H_INCLUDE_SIMPLE


// Add a reception delay, if needed.
// This may be needed for a fast processor at a slow baud rate.
// #define TINY_GSM_YIELD() { delay(2); }

struct Location {
  float lon;
  float lat;
};

struct Point {
  int x;
  int y;
};

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

