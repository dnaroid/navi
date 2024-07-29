#include <PNGdec.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <TinyGPSPlus.h>
#include <Wire.h>

#include "lib/globals.h"
#include "lib/coord.h"
#include "lib/NS2009.h"
#include "lib/png.h"

auto spiDisplay = SPIClass(FSPI);
auto spiSD = SPIClass(HSPI);

TinyGPSPlus gps;
HardwareSerial gpsSerial(1);


// const
Rect BTN_PLUS_RECT = {10, 10, 50, 50};
Rect BTN_MINUS_RECT = {10, 70, 50, 50};
Rect BTN_LOCATION_RECT = {10, 130, 50, 50};

// global vars
int zoom = 18;
int angle = 0;
int new_angle = 0;
double center_lat = 54.3926814;
double center_lon = 18.6209547;
double my_lat = 0;
double my_lon = 0;
double end_point_lat = 0.0;
double end_point_lon = 0.0;
int mouse_down_time = 0;
bool isDragging = false;
int last_drag_x = 0;
int last_drag_y = 0;
char btnPressed = '\0';
int touch_pos[2] = {0, 0};
unsigned int btnPressedAt = 0;

static char path_name[MAX_FILENAME_LENGTH];

const char* getTilePath(int z, int x, int y) {
  std::snprintf(path_name, sizeof(path_name), "/tiles/%d/%d/%d.png", z, x, y);
  // print(path_name);
  return path_name;
}

void drawTile(int x, int y, int sx, int sy) {
  int x2 = sx + TILE_SIZE;
  int y2 = sy + TILE_SIZE;
  if (sx <= SCREEN_WIDTH && x2 >= 0 && sy <= SCREEN_HEIGHT && y2 >= 0) {
    drawPngTile(getTilePath(zoom, x, y), sx, sy);
  }
}

void drawButton(const Rect* rect, const char* text) {
  TFT.drawRect(rect->x, rect->y, rect->w, rect->h, btnPressed == *text ? TFT_WHITE : TFT_BLACK);
  TFT.drawString(text, rect->x + 20, rect->y + 12, 4);
}

void drawButtons() {
  if (zoom < 18) drawButton(&BTN_PLUS_RECT, "+");
  if (zoom > 12) drawButton(&BTN_MINUS_RECT, "-");
  if (my_lat && my_lon) drawButton(&BTN_LOCATION_RECT, "?");
}

void drawMyMarker() {
  TFT.setCursor(0, 20);
  TFT.fillRect(0, 0, 100, 40,TFT_WHITE);
  TFT.print(angle);

  // if (!my_lon || !my_lat) { return; }
  int x = lon_to_screen(my_lon, center_lon, zoom);
  int y = lat_to_screen(my_lat, center_lat, zoom);
  if (!my_lon || !my_lat) { // todo debug
    x = SCREEN_CENTER_X;
    y = SCREEN_CENTER_Y;
  }
  TFT.fillCircle(x, y, MY_MARKER_R,TFT_BLUE);

  float angle_rad = angle * DEG_TO_RAD;
  int offsetX = (MY_MARKER_R - MY_MARKER_R2 - 1) * cos(angle_rad);
  int offsetY = -(MY_MARKER_R - MY_MARKER_R2 - 1) * sin(angle_rad);
  TFT.fillCircle(x + offsetX, y + offsetY, MY_MARKER_R2, TFT_WHITE);
}

void drawMap() {
  int x_pixel_current = lon_to_pixels(center_lon, zoom);
  int y_pixel_current = lat_to_pixels(center_lat, zoom);
  int x_tile = x_pixel_current / TILE_SIZE;
  int y_tile = y_pixel_current / TILE_SIZE;
  int x_offset = x_pixel_current % TILE_SIZE;
  int y_offset = y_pixel_current % TILE_SIZE;
  int tile_screen_x = SCREEN_CENTER_X - x_offset;
  int tile_screen_y = SCREEN_CENTER_Y - y_offset;
  for (int dx : TILES_X_SCAN) {
    for (int dy : TILES_Y_SCAN) {
      drawTile(x_tile + dx, y_tile + dy, tile_screen_x + dx * TILE_SIZE, tile_screen_y + dy * TILE_SIZE);
      if (dx < 1 && dy < 1) drawButtons();
    }
  }
  drawMyMarker();
}

bool inRect(int px, int py, const Rect* rect) {
  return (px >= rect->x && px <= (rect->x + rect->w) && py >= rect->y && py <= (rect->y + rect->h));
}

void setup() {
  delay(300);
  Serial.begin(9600);
  int waitCount = 0;
  while (!Serial && waitCount < 30) { // 3 seconds
    delay(100);
    waitCount++;
  }

  delay(300);
  print("--------------- started ---------------");

  spiDisplay.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
  TFT.init();
  TFT.setRotation(2);
  TFT.fillScreen(TFT_BLACK);

  TFT.setTextColor(TFT_WHITE);
  TFT.setFreeFont(&FreeMono9pt7b);
  TFT.setCursor(0, 20);
  TFT.print("Init card reader");
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  int frq = 80000000;
  while (!SD.begin(SD_CS, spiSD, frq) && frq > 1000000) {
    frq -= 1000000;
    delay(100);
    TFT.print(".");
  }
  TFT.println("");
  TFT.print("SD speed:");
  print("SD speed:", frq);
  TFT.println(frq);
  TFT.println("Init SD card");

  while (SD.cardType() == CARD_NONE) {
    TFT.print(".");
    print("No SD card attached");
    delay(500);
  }
  TFT.println("SD card is OK");
  TFT.println("Init touch");

  if (initTouch()) {
    TFT.println("Touch is OK");
  } else {
    TFT.println("Init touch fail");
  }

  TFT.println("Init GPS");
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  TFT.println("Init compass");

  Wire.begin(I2C_SDA,I2C_SCL,0);

  TFT.println("Init cache");
  initializeCache();
  TFT.println("Init done!");

  TFT.setTextColor(TFT_BLUE);
  drawMap();
}

void loop() {

  // new_angle = compass.getAzimuth();
  // // if (abs(new_angle - angle) > COMPASS_ANGLE_STEP) {
  // angle = new_angle;
  // drawMyMarker();
  // // }

  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isUpdated()) {
    double gps_lat = gps.location.lat();
    double gps_lon = gps.location.lng();
    if (std::abs(my_lat - gps_lat) > MIN_COORD_CHANGE || (std::abs(my_lon - gps_lon) > MIN_COORD_CHANGE)) {
      my_lat = gps_lat;
      my_lon = gps_lon;
      drawMap();
    }
  }

  if (!ns2009_pos(touch_pos)) { // released
    if (isDragging) { isDragging = false; }
  } else { // pressed
    int px = touch_pos[0];
    int py = touch_pos[1];

    // buttons pressed?
    if (inRect(px, py, &BTN_PLUS_RECT)) {
      if (zoom < 18) {
        zoom++;
        btnPressed = '+';
        btnPressedAt = millis();
        drawMap();
      }
    } else if (inRect(px, py, &BTN_MINUS_RECT)) {
      if (zoom > 12) {
        zoom--;
        btnPressed = '-';
        btnPressedAt = millis();
        drawMap();
      }
    } else if (inRect(px, py, &BTN_LOCATION_RECT)) {
      zoom = 18;
      btnPressed = '?';
      center_lat = my_lat;
      center_lon = my_lon;
      btnPressedAt = millis();
      drawMap();
    } else {
      // map pressed
      if (isDragging) {
        int dx = px - last_drag_x;
        int dy = py - last_drag_y;
        last_drag_x = px;
        last_drag_y = py;
        int x_pixel_start = lon_to_pixels(center_lon, zoom);
        int y_pixel_start = lat_to_pixels(center_lat, zoom);
        int x_pixel_new = x_pixel_start - dx;
        int y_pixel_new = y_pixel_start - dy;
        center_lat = pixels_to_lat(y_pixel_new, zoom);
        center_lon = pixels_to_lon(x_pixel_new, zoom);
        drawMap();
      } else { // start drag
        last_drag_x = px;
        last_drag_y = py;
        isDragging = true;
      }
    }
  }

  // clear btn pressed state
  if (millis() - btnPressedAt > BUTTON_PRESS_ANIM_TIME) {
    btnPressed = '\0';
    drawButtons();
  }
  // delay(1);
}
