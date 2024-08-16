#include "PolChar.h"
#include "Arduino.h"

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
