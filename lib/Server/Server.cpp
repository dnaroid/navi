#include <Arduino.h>
#include <esp_wifi.h>
#include <globals.h>
#include <TJpg_Decoder.h>
#include "secrets.h"
#include <WiFi.h>
#include <WebSocketsServer.h>


static WebSocketsServer webSocket = WebSocketsServer(81);

#define USE_SERIAL Serial

static bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  // tft.pushImage(x, y, w, h, bitmap);
  return true;
}

static void hexdump(const void* mem, uint32_t len, uint8_t cols = 16) {
  const uint8_t* src = (const uint8_t*)mem;
  USE_SERIAL.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for (uint32_t i = 0; i < len; i++) {
    if (i % cols == 0) {
      USE_SERIAL.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    USE_SERIAL.printf("%02X ", *src);
    src++;
  }
  USE_SERIAL.printf("\n");
}

static void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    USE_SERIAL.printf("[%u] Disconnected!\n", num);
    break;
  case WStype_CONNECTED: {
    IPAddress ip = webSocket.remoteIP(num);
    USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

    // send message to client
    webSocket.sendTXT(num, "Connected");
  }
  break;
  case WStype_TEXT:
    USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

  // send message to client
  // webSocket.sendTXT(num, "message here");

  // send data to all connected clients
  // webSocket.broadcastTXT("message here");
    break;
  case WStype_BIN:
    TJpgDec.drawJpg(0, 0, payload, length);
  // USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
  // hexdump(payload, length);

  // send message to client
  // webSocket.sendBIN(num, payload, length);
    break;
  case WStype_ERROR:
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
    break;
  }
}

void ServerSetup() {
  LOGI("Init AP ");
  esp_wifi_set_max_tx_power(1);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  LOG("IP: ", WiFi.softAPIP());

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  webSocket.onEvent(webSocketEvent);
  webSocket.begin();
}

void ServerLoop() {
  webSocket.loop();
}
