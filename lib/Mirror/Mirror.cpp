#ifdef MIRROR

#include <esp_wifi.h>
#include <globals.h>
#include <Mirror.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include "WebSocketsServer.h"

extern TFT_eSPI* tft_inst;
extern SemaphoreHandle_t xGuiSemaphore;
// global
bool camEnabled = false;
int camWsClientNumber = -1;
static int mirror_width = 96;
static int mirror_height = 96;

static WebSocketsServer webSocket(81);
static bool serverReady = false;

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
  int commaIndex;
  String receivedData;
  switch (type) {
  case WStype_DISCONNECTED:
    LOGF("[%u] Disconnected\n", num);
    camWsClientNumber = -1;
    break;
  case WStype_CONNECTED:
    LOGF("[%u] Connected\n", num);
    camWsClientNumber = num;
    if (camEnabled) {
      webSocket.sendTXT(num, "start");
    } else {
      webSocket.sendTXT(num, "stop");
    }
    break;
  case WStype_BIN:
    if (camEnabled) TJpgDec.drawJpg(SCREEN_CENTER_X - mirror_width / 2, SCREEN_CENTER_Y - mirror_height / 2, payload, length);
    break;
  case WStype_TEXT:
    receivedData = String((char*)payload);
    commaIndex = receivedData.indexOf(',');
    if (commaIndex != -1) {
      String widthStr = receivedData.substring(0, commaIndex);
      String heightStr = receivedData.substring(commaIndex + 1);
      mirror_width = widthStr.toInt();
      mirror_height = heightStr.toInt();
    }
    break;
  default:
    break;
  }
}

void Mirror_init() {
  LOGI("Init Mirror");
  TJpgDec.setJpgScale(1);
  esp_wifi_set_max_tx_power(1);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  LOGI("WS server IP:", WiFi.softAPIP());
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  serverReady = true;
  LOG(" ok");
}

void Mirror_start() {
  if (!serverReady) Mirror_init();
  camEnabled = true;
  LOG("[Mirror.cpp] cam enabled");
  if (camWsClientNumber != -1) webSocket.sendTXT(camWsClientNumber, "start");
}

void Mirror_stop() {
  camEnabled = false;
  LOG("[Mirror.cpp] cam disabled");
  if (camWsClientNumber != -1) webSocket.sendTXT(camWsClientNumber, "stop");
  webSocket.disconnect();
  webSocket.close();
  WiFi.disconnect();
  serverReady = false;
}

void Mirror_loop() {
  if (serverReady) webSocket.loop();
}

#endif
