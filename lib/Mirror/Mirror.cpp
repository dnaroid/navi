#ifdef MIRROR

#include <globals.h>
#include <Mirror.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

#ifdef MIRROR_UART
#define UART_MIRROR_BUF_SIZE 3000
uint8_t buf[UART_MIRROR_BUF_SIZE];
bool isBufReady;
HardwareSerial mirrorSerial(2);
#else
#include <esp_wifi.h>
#include "WebSocketsServer.h"
static WebSocketsServer webSocket(81);
static bool serverReady = false;
int camWsClientNumber = -1;
#endif

extern TFT_eSPI* tft_inst;
extern SemaphoreHandle_t xGuiSemaphore;
// global
bool camEnabled = false;

static bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  // if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE) {
  tft_inst->startWrite();
  tft_inst->setAddrWindow(x, y, w, h);
  tft_inst->pushColors(bitmap, w * h, false);
  tft_inst->endWrite();
  isBufReady = true;
  //   xSemaphoreGive(xGuiSemaphore);
  //   return true;
  // }
  return true;
}

#ifndef MIRROR_UART
static void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
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
    if (camEnabled) TJpgDec.drawJpg(SCREEN_CENTER_X - 160 / 2, SCREEN_HEIGHT - 120, payload, length);
    break;
  default:
    break;
  }
}
#endif

void Mirror_init() {
  LOGI("Init Mirror");
#ifdef MIRROR_UART
#else
  esp_wifi_set_max_tx_power(1);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  LOGI("WS server IP:", WiFi.softAPIP());
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  serverReady = true;
#endif
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
  LOG(" ok");
}

void Mirror_start() {
#ifdef MIRROR_UART
  mirrorSerial.begin(MIRROR_UART_BAUD, SERIAL_8N1, MIRROR_RX, MIRROR_TX);
  camEnabled = true;
  isBufReady = true;
  mirrorSerial.print("img\r");
#else
  camEnabled = true;
  if (!serverReady) Mirror_init();
  LOG("[Mirror.cpp] cam enabled");
  if (camWsClientNumber != -1) webSocket.sendTXT(camWsClientNumber, "start");
#endif
}

void Mirror_stop() {
  isBufReady = false;
  camEnabled = false;
#ifdef MIRROR_UART
  mirrorSerial.end();
#else
  LOG("[Mirror.cpp] cam disabled");
  if (camWsClientNumber != -1) webSocket.sendTXT(camWsClientNumber, "stop");
#endif
}

int buf_skips = 0;
uint16_t length;

void Mirror_loop() {
#ifdef MIRROR_UART
  if (!camEnabled) return;
  if (mirrorSerial.available() > 1) {
    mirrorSerial.print("img\r");
    mirrorSerial.readBytes(reinterpret_cast<uint8_t*>(&length), sizeof(length));
    auto readed = mirrorSerial.readBytes(buf, length);
    if (readed < length)
      LOG("[125:Mirror.cpp] length:", length, readed);
    // TJpgDec.drawJpg(SCREEN_CENTER_X - 160 / 2, SCREEN_HEIGHT - 120, buf, length);
    TJpgDec.drawJpg(0, 0, buf, length);
  }
  delay(1000);
#else
  if (serverReady) webSocket.loop();
#endif
}

#endif
