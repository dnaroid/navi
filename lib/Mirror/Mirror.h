#ifndef MIRROR_H
#define MIRROR_H

#include "globals.h"
#include <TJpg_Decoder.h>
#include <HTTPClient.h>

HTTPClient http;

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  TFT.pushImage(x, y, w, h, bitmap);
  return true;
}

class Mirror {
public:
  Mirror() {
    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tft_output);
  }

  void connect() {
    LOG("Init cam");
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      LOGI(".");
    }
    LOG("\nESP32-S3 IP Address: ", WiFi.localIP());
    if (http.begin("http://192.168.4.1/jpg")) {
      const int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK) {
        LOG("Cam is ok");
      } else {
        LOG("Failed to connect, error: ", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      LOG("Failed to initialize HTTPClient");
    }
  }

  void disconnect() {
    WiFi.mode(WIFI_OFF);
    WiFi.disconnect(true);
    LOG("WiFi turned off.");
  }

  void drawImage(const int x, const int y) {
    HTTPClient http;
    if (http.begin("http://192.168.4.1/jpg")) {
      const int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK) {
        const auto payload = reinterpret_cast<const uint8_t*>(http.getString().c_str());
        TJpgDec.drawJpg(x, y, payload, http.getSize());
      } else {
        LOG("Failed to get image, error: ", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      LOG("Failed to initialize HTTPClient");
    }
  }
};


#endif //MIRROR_H
