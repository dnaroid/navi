#ifndef GLOBALS_H
#define GLOBALS_H

#include "TFT_eSPI.h"

/// --- features
// #define DISABLE_TFT
// #define DISABLE_UI
// #define DISABLE_TOUCH
// #define DISABLE_SD
#define DISABLE_DB
#define DISABLE_TILE_CACHE
#define DISABLE_SERVER
#define DISABLE_COMPASS
#define DISABLE_GPS

#define TINY_GSM_MODEM_SIM800

/// --- pins
#define SD_CS 18
#define SD_MOSI 17
#define SD_SCK 16
#define SD_MISO 15

#define TFT_MOSI 11
#define TFT_MISO 13
#define TFT_SCLK 12
#define TFT_CS 10
#define TFT_DC 14

#define I2C_SDA 8
#define I2C_SCL 9

#define GPS_RX 47
#define GPS_TX 48

// #define A9G_PON 4
// #define A9G_LOWP 5
// #define A9G_RST 6

/// --- screen
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480
#define SCREEN_CENTER_X 160
#define SCREEN_CENTER_Y 240
#define TILE_SIZE 256
#define TILES_X_SCAN {0,1,-1,2,-2}
#define TILES_Y_SCAN {0,1,-1,2,-2}
#define BUTTON_PRESS_ANIM_TIME 500
#define MY_MARKER_R 10
#define MY_MARKER_R2 3
#define BUTTON_W 32
#define BUTTON_H 32
#define BUTTON_SPACING 0
#define KEYBOARD_Y 351
#define KEYBOARD_X 0
#define KEYBOARD "1234567890qwertyuiopasdfghjkl<zxcvbnm  >"

/// --- compass
#define COMPASS_ANGLE_STEP 10
#define COMPASS_ANGLE_CORRECTION 0
#define COMPASS_MAGNETIC_DECLINATION_D 6
#define COMPASS_MAGNETIC_DECLINATION_M 43

/// --- cache
// #define MAX_CACHE_SIZE 15
#define MAX_CACHE_SIZE 9
#define MAX_FILENAME_LENGTH 35

/// --- timing
#define COMPASS_UPDATE_PERIOD 1000
#define GPS_UPDATE_PERIOD 10000

/// --- GPS
#define MIN_COORD_CHANGE 0.0001

/// --- GPRS
#define MODEM_TX 4
#define MODEM_RX 5

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

struct Address {
  String name;
  Location location;
};

extern TFT_eSPI TFT;
extern TFT_eSprite mapSprite;

inline void print() {
}

template <typename T, typename... Args>
void print(T first, Args... args) {
  Serial.print(first);
  print(args...);
}

template <typename... Args>
void println(Args... args) {
  print(args...);
  Serial.println();
}


#ifdef RELEASE
#define LOG(...)
#define LOGI(...)
#define LOGF(...)
#else
#define LOG(...) println(__VA_ARGS__)
#define LOGI(...) print(__VA_ARGS__)
#define LOGF(...) Serial.printf(__VA_ARGS__)
#endif

#define delay(ms) vTaskDelay(ms / portTICK_PERIOD_MS)


#endif //GLOBALS_H
