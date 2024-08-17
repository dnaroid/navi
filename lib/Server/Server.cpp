#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <esp_wifi.h>
#include <globals.h>
#include <Secrets.h>
#include <TJpg_Decoder.h>

AsyncWebServer server(80);

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  TFT.pushImage(x, y, w, h, bitmap);
  return true;
}

void handleRoot(AsyncWebServerRequest* request) {
  request->send(200, "text/plain", "Hello from ESP32-S3!");
}

void handleJpgPost(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
  // LOG(len, " ", index, " ", final);
  static uint8_t* imageBuffer = nullptr;
  static size_t imageSize = 0;

  if (index == 0) {
    imageSize = len;
    imageBuffer = new uint8_t[imageSize];
    memcpy(imageBuffer, data, len);
  } else {
    uint8_t* tempBuffer = new uint8_t[imageSize + len];
    memcpy(tempBuffer, imageBuffer, imageSize);
    delete[] imageBuffer;
    imageBuffer = tempBuffer;
    imageSize += len;
    memcpy(imageBuffer + imageSize - len, data, len);
  }

  if (final) {
    TJpgDec.drawJpg(0, 0, imageBuffer, imageSize);
    delete[] imageBuffer;
    imageBuffer = nullptr;
  }

  request->send(200, "text/plain", "Image received");
}

void handleNotFound(AsyncWebServerRequest* request) {
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
  request->send(404, "text/plain", message);
}

void handleJpgUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
  TJpgDec.drawJpg(0, 0, data, len);

  if (final) {
    request->send(200, "text/plain", "Image upload successful");
  }
}

void ServerSetup() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_max_tx_power(1);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  LOG("Access Point IP: ", WiFi.softAPIP());

  server.on("/", HTTP_GET, handleRoot);

  server.on("/jpg", HTTP_POST, [](AsyncWebServerRequest* request) {
  }, handleJpgUpload);

  server.onNotFound(handleNotFound);

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  server.begin();
  LOG("HTTP server started");
}
