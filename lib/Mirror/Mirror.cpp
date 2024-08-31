#include "Mirror.h"

#include <globals.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

#include "WebSocketsServer.h"

static WebSocketsServer webSocket(81);
extern TFT_eSPI* tft_inst;
static bool camEnabled = false;
static int camNum = -1;

static bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  tft_inst->pushImage(x, y, w, h, bitmap);
  return true;
}

static void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    LOGF("[%u] Disconnected\n", num);
    camNum = -1;
    break;
  case WStype_CONNECTED:
    LOGF("[%u] Connected\n", num);
    camNum = num;
    if (camEnabled) webSocket.sendTXT(num, "start");
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
  LOG("WS server IP:", WiFi.softAPIP());
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void Mirror_start() {
  camEnabled = true;
  if (camNum != -1) webSocket.sendTXT(camNum, "start");
}

void Mirror_stop() {
  camEnabled = false;
  if (camNum != -1) webSocket.sendTXT(camNum, "stop");
}

void Mirror_loop() {
  webSocket.loop();
}
