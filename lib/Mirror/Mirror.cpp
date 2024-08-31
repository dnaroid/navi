#include <esp_wifi.h>
#include <globals.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include "WebSocketsServer.h"

extern SemaphoreHandle_t xGuiSemaphore;

static WebSocketsServer webSocket(81);
extern TFT_eSPI* tft_inst;
bool camEnabled = false;
int camNum = -1;

static bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE) {
    tft_inst->startWrite();
    tft_inst->setAddrWindow(x, y, w, h);
    tft_inst->pushColors(bitmap, w * h, false);
    tft_inst->endWrite();
    xSemaphoreGive(xGuiSemaphore);
    return true;
  }
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
    if (camEnabled) {
      webSocket.sendTXT(num, "start");
    } else {
      webSocket.sendTXT(num, "stop");
    }
    break;
  case WStype_BIN:
    if (camEnabled) TJpgDec.drawJpg(0, SCREEN_HEIGHT - 120, payload, length);
    break;
  default:
    break;
  }
}

void Mirror_init() {
  TJpgDec.setJpgScale(1);
  esp_wifi_set_max_tx_power(1);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
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

void Mirror_toggle() {
  if (camNum == -1) return;
  if (camEnabled) {
    Mirror_stop();
  } else {
    Mirror_start();
  }
}

void Mirror_loop() {
  webSocket.loop();
}
