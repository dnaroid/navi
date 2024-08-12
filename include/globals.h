#ifndef GLOBALS_H
#define GLOBALS_H

#include "secrets.h"
#include "TFT_eSPI.h"

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
// #define CACHE_DISABLED
#ifndef CACHE_DISABLED
#define MAX_CACHE_SIZE 15
#endif
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
#define CAM_URL "http://192.168.4.1/jpg"

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

inline void print() {
}

template <typename T, typename... Args>
void print(T first, Args... args) {
  Serial.print(first);
  Serial.print(' ');
  print(args...);
  Serial.println();
}

#define delay(ms) vTaskDelay(ms / portTICK_PERIOD_MS)

#define UTF8TOWIN1250_BUFFER_LENGTH 500
char utf8ToWin1250_buffer[UTF8TOWIN1250_BUFFER_LENGTH];

char* utf8(const char* str) {
  uint16_t idx = 0;
  uint8_t last = 0;
  while (((*str) != 0) && (idx < UTF8TOWIN1250_BUFFER_LENGTH - 1)) {
    if (((uint8_t)*str) < 128) {
      last = 0;
      utf8ToWin1250_buffer[idx++] = *str;
    } else if ((((uint8_t)*str) & 0xC0) == 0xC0) {
      last = ((uint8_t)*str);
    } else if ((last & 0xC0) == 0xC0) {
      uint16_t word = (last << 8) | ((uint8_t)*str);
      uint8_t character;
      switch (word) {
      case 0xC484: character = 0x1;
        break; // Ą

      case 0xC486: character = 0x2;
        break; // Ć

      case 0xC498: character = 0x3;
        break; // Ę

      case 0xC581: character = 0x4;
        break; // Ł

      case 0xC583: character = 0x5;
        break; // Ń

      case 0xC393: character = 0x6;
        break; // Ó

      case 0xC59A: character = 0x7;
        break; // Ś

      case 0xC5B9: character = 0x8;
        break; // Ź

      case 0xC5BB: character = 0x9;
        break; // Ż

      case 0xC485: character = 0x13;
        break; // ą

      case 0xC487: character = 0xb;
        break; // ć

      case 0xC499: character = 0xc;
        break; // ę

      case 0xC582: character = 0x14;
        break; // ł

      case 0xC584: character = 0xe;
        break; // ń

      case 0xC3B3: character = 0xF;
        break; // ó

      case 0xC59B: character = 0x10;
        break; // ś

      case 0xC5BA: character = 0x11;
        break; // ź

      case 0xC5BC: character = 0x12;
        break; // ż

      default: character = 0;
      }
      if (character != 0) {
        utf8ToWin1250_buffer[idx++] = character;
      }
      last = 0;
    } else {
      utf8ToWin1250_buffer[idx++] = *str;
    }
    str++;
  }
  utf8ToWin1250_buffer[idx++] = 0;
  return utf8ToWin1250_buffer;
}

#endif //GLOBALS_H
