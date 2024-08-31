#include "Mirror.h"

#include <globals.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

#include "WebSocketsServer.h"

WebSocketsServer webSocket(81);
extern TFT_eSPI* tft_inst;


static bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  tft_inst->pushImage(x, y, w, h, bitmap);
  return true;
}

static void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected\n", num);
    break;
  case WStype_CONNECTED:
    Serial.printf("[%u] Connected\n", num);
    webSocket.sendTXT(num, "image");
    break;
  case WStype_BIN:
    TJpgDec.drawJpg(0, 0, payload, length);
    break;
  default:
    break;
  }
}

void Mirror_init() {
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void Mirror_start() {
  webSocket.broadcastTXT("camera.start");
}

void Mirror_stop() {
  webSocket.broadcastTXT("camera.stop");
}

void Mirror_loop() {
  webSocket.loop();
}
