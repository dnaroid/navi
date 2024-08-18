#include <TFT_eSPI.h>

auto TFT = TFT_eSPI();
auto mapSprite = TFT_eSprite(&TFT);

bool isReadySD = false;
bool isReadyUI = false;
bool isReadyDB = false;
bool isReadyGPS = false;
bool isReadyWiFi = false;
bool isReadyCompass = false;
bool isReadyCamera = false;
