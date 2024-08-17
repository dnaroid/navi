#include <esp_wifi.h>

#include "globals.h"
#include <TJpg_Decoder.h>
#include <HTTPClient.h>
#include <Mirror.h>


bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  TFT.pushImage(x, y, w, h, bitmap);
  return true;
}

bool MirrorInit() {
  LOG("Init Cam");
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  // WiFi.mode(WIFI_MODE_STA);
  esp_wifi_set_max_tx_power(1);

  // return true;

  LOGI("     connect to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    LOGI(".");
  }
  LOG(" ok");
  return true;
  LOGI("     take image");
  http.begin(wifi,IMAGE_CAPTURE_URL);
  const int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    LOG(" ok");
    http.end();
    return true;
  }
  LOG(" fail: ", http.errorToString(httpCode).c_str());
  return false;
}

bool MirrorConnect() {
  LOG("Connect to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 50;
  while (WiFi.status() != WL_CONNECTED && --attempts > 0) {
    delay(100);
    LOGI(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    LOG("Failed to connect WiFi");
    return false;
  }
}

void MirrorDisconnect() {
  WiFi.mode(WIFI_OFF);
  WiFi.disconnect(true);
  LOG("WiFi turned off");
}

bool MirrorDraw(const int x, const int y) {
  if (WiFi.status() != WL_CONNECTED) MirrorConnect();
  if (!http.connected()) return false;
  HTTPClient http;
  WiFiClient wifi;
  if (http.begin(wifi,IMAGE_CAPTURE_URL)) {
    // http.addHeader("Connection", "keep-alive");
    const int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      const uint8_t* payload = (uint8_t*)http.getString().c_str();
      TJpgDec.drawJpg(x, y, payload, http.getSize());
    } else {
      LOG("Failed to get image, error: ", http.errorToString(httpCode).c_str());
      return false;
    }
    http.end();
    return true;
  }
  LOG("Failed to initialize HTTPClient");
  return false;
}
