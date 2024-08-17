#include <HTTPClient.h>
#include <TFT_eSPI.h>

auto TFT = TFT_eSPI();

HTTPClient http;
WiFiClient wifi;

bool isReadySD = false;
bool isReadyUI = false;
bool isReadyDB = false;
bool isReadyGPS = false;
bool isReadyWiFi = false;
bool isReadyCompass = false;
bool isReadyCamera = false;
