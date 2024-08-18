#include "globals.h"
#include "PNGdec.h"
#include "SD.h"
#include "SPI.h"
#include "Wire.h"
#include "TFT_eSPI.h"
#include "WiFi.h"
#include "coord.h"
#include "PngTile.h"
#include "LSM303.h"
#include "UI.h"
#include "Touch.h"
#include "sqlite3.h"
#include "PathFinder.h"
#include "lvgl.h"
#include "../lv_conf.h"
#include "demos/lv_demos.h"

sqlite3* addrDb;
Touch touch;
UI ui;
LSM303 compass;
PathFinder pathFinder;

char* zErrMsg = nullptr;
int rc;
const char* dbData = "Callback function called";
std::vector<Address> foundAddrs; //todo make class

// global vars

int zoom = 14;
int angle = 0;
int new_angle = 0;
float init_lat = 54.3926814;
float init_lon = 18.6209547;
Location centerLoc = {init_lon, init_lat};
Location targetLoc = {0, 0};
Location myLoc = centerLoc;
unsigned int now;
bool isKeyboardActive = false;
bool isShowAddresses = false;
unsigned long compassUpdateAfterMs;
unsigned long gpsUpdateAfterMs;

static char path_name[MAX_FILENAME_LENGTH];

auto spiSD = SPIClass(VSPI);


// LVGL ------------------------------------------------

// Колбэк для первой кнопки
void btn1_event_cb(lv_event_t* e) {
  Serial.println("Button 1 pressed");
}

// Колбэк для второй кнопки
void btn2_event_cb(lv_event_t* e) {
  Serial.println("Button 2 pressed");
}

int btn1_count = 0;
// Callback that is triggered when btn1 is clicked
static void event_handler_btn1(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    btn1_count++;
    LV_LOG_USER("Button clicked %d%", (int)btn1_count);
  }
}

// Callback that is triggered when btn2 is clicked/toggled
static void event_handler_btn2(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    LV_UNUSED(obj);
    LV_LOG_USER("Toggled %s", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "on" : "off");
  }
}

static lv_obj_t* slider_label;
// Callback that prints the current slider value on the TFT display and Serial Monitor for debugging purposes
static void slider_event_callback(lv_event_t* e) {
  lv_obj_t* slider = (lv_obj_t*)lv_event_get_target(e);
  char buf[8];
  lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
  lv_label_set_text(slider_label, buf);
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  LV_LOG_USER("Slider changed to %d%%", (int)lv_slider_get_value(slider));
}

void lv_create_main_gui() {
  // Create a text label aligned center on top ("Hello, world!")
  lv_obj_t* text_label = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(text_label, LV_LABEL_LONG_WRAP); // Breaks the long lines
  lv_label_set_text(text_label, "Hello, world!");
  lv_obj_set_width(text_label, 150); // Set smaller width to make the lines wrap
  lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(text_label, LV_ALIGN_CENTER, 0, -90);

  lv_obj_t* btn_label;
  // Create a Button (btn1)
  lv_obj_t* btn1 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn1, event_handler_btn1, LV_EVENT_ALL, NULL);
  lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -50);
  lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

  btn_label = lv_label_create(btn1);
  lv_label_set_text(btn_label, "Button");
  lv_obj_center(btn_label);

  // Create a Toggle button (btn2)
  lv_obj_t* btn2 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn2, event_handler_btn2, LV_EVENT_ALL, NULL);
  lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 10);
  lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_height(btn2, LV_SIZE_CONTENT);

  btn_label = lv_label_create(btn2);
  lv_label_set_text(btn_label, "Toggle");
  lv_obj_center(btn_label);

  // Create a slider aligned in the center bottom of the TFT display
  lv_obj_t* slider = lv_slider_create(lv_screen_active());
  lv_obj_align(slider, LV_ALIGN_CENTER, 0, 60);
  lv_obj_add_event_cb(slider, slider_event_callback, LV_EVENT_VALUE_CHANGED, NULL);
  lv_slider_set_range(slider, 0, 100);
  lv_obj_set_style_anim_duration(slider, 2000, 0);

  // Create a label below the slider to display the current slider value
  slider_label = lv_label_create(lv_screen_active());
  lv_label_set_text(slider_label, "0%");
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

void create_demo_ui() {
  // Создаем первую кнопку
  lv_obj_t* btn1 = lv_btn_create(lv_scr_act()); // Создаем кнопку на активном экране
  lv_obj_set_size(btn1, 100, 50); // Устанавливаем размер кнопки
  lv_obj_align(btn1, LV_ALIGN_CENTER, -60, 0); // Выравниваем кнопку слева от центра
  lv_obj_add_event_cb(btn1, btn1_event_cb, LV_EVENT_CLICKED, NULL); // Устанавливаем колбэк на нажатие

  lv_obj_t* label1 = lv_label_create(btn1); // Создаем метку на кнопке
  lv_label_set_text(label1, "Button 1"); // Устанавливаем текст метки

  // Создаем вторую кнопку
  lv_obj_t* btn2 = lv_btn_create(lv_scr_act()); // Создаем кнопку на активном экране
  lv_obj_set_size(btn2, 100, 50); // Устанавливаем размер кнопки
  lv_obj_align(btn2, LV_ALIGN_CENTER, 60, 0); // Выравниваем кнопку справа от центра
  lv_obj_add_event_cb(btn2, btn2_event_cb, LV_EVENT_CLICKED, NULL); // Устанавливаем колбэк на нажатие

  lv_obj_t* label2 = lv_label_create(btn2); // Создаем метку на кнопке
  lv_label_set_text(label2, "Button 2"); // Устанавливаем текст метки
}

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];
#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char* buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
  TFT.pushImage(area->x1, area->y1, lv_area_get_width(area), lv_area_get_height(area), px_map);
  lv_display_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t* indev, lv_indev_data_t* data) {
  int touch_pos[2] = {0, 0};
  bool touched = ns2009_pos(touch_pos);

  if (!touched) {
    data->state = LV_INDEV_STATE_RELEASED;
  } else {
    data->state = LV_INDEV_STATE_PRESSED;

    data->point.x = touch_pos[0];
    data->point.y = touch_pos[1];
  }
}

/*use Arduinos millis() as tick source*/
static uint32_t my_tick(void) {
  return millis();
}


const char* getTilePath(int z, int x, int y) {
  std::snprintf(path_name, sizeof(path_name), "/tiles/%d/%d/%d.png", z, x, y);
  return path_name;
}

inline void drawTile(int x, int y, int sx, int sy) {
  int x2 = sx + TILE_SIZE;
  int y2 = sy + TILE_SIZE;
  if (sx <= SCREEN_WIDTH && x2 >= 0 && sy <= SCREEN_HEIGHT && y2 >= 0) {
    drawPngTile(getTilePath(zoom, x, y), sx, sy);
  }
}

void drawMyMarker() {
  if (!myLoc.lat) { return; }
  Point p = coord::locationToScreen(myLoc, centerLoc, zoom);
  TFT.fillCircle(p.x, p.y, MY_MARKER_R,TFT_BLUE);
  float angle_rad = angle * DEG_TO_RAD;
  int offsetX = (MY_MARKER_R - MY_MARKER_R2 - 1) * cos(angle_rad);
  int offsetY = -(MY_MARKER_R - MY_MARKER_R2 - 1) * sin(angle_rad);
  TFT.fillCircle(p.x + offsetX, p.y + offsetY, MY_MARKER_R2, TFT_WHITE);
}

void drawTargetMarker() {
  if (!targetLoc.lon) { return; }
  constexpr int radius = 10;
  constexpr int dy = -(radius * 1.5);
  constexpr int color = TFT_BLUE;
  Point p = coord::locationToScreen(targetLoc, centerLoc, zoom);
  TFT.fillCircle(p.x, p.y + dy, radius, color);
  TFT.fillTriangle(p.x - radius, p.y + dy, p.x + radius, p.y + dy, p.x, p.y, color);
  TFT.fillCircle(p.x, p.y + dy, radius / 3, TFT_WHITE);
}

void drawRoute() {
  const auto route = pathFinder.path;
  if (route.empty()) { return; }
  Point p1 = coord::locationToScreen(route[0], centerLoc, zoom);
  for (size_t i = 1; i < route.size(); i++) {
    const Point p2 = coord::locationToScreen(route[i], centerLoc, zoom);
    TFT.drawLine(p1.x, p1.y, p2.x, p2.y,TFT_BLUE);
    TFT.drawLine(p1.x, p1.y + 1, p2.x, p2.y + 1,TFT_BLUE);
    TFT.drawLine(p1.x, p1.y - 1, p2.x, p2.y - 1,TFT_BLUE);
    TFT.drawLine(p1.x + 1, p1.y, p2.x + 1, p2.y,TFT_BLUE);
    TFT.drawLine(p1.x - 1, p1.y, p2.x - 1, p2.y,TFT_BLUE);
    p1 = p2;
  }
}

void drawMap() {
  unsigned long startMillis = millis();
  Point p = coord::locationToPixels(centerLoc, zoom);
  int x_tile = p.x / TILE_SIZE;
  int y_tile = p.y / TILE_SIZE;
  int x_offset = p.x % TILE_SIZE;
  int y_offset = p.y % TILE_SIZE;
  int tile_screen_x = SCREEN_CENTER_X - x_offset;
  int tile_screen_y = SCREEN_CENTER_Y - y_offset;
  for (int dx : TILES_X_SCAN) {
    for (int dy : TILES_Y_SCAN) {
      drawTile(x_tile + dx, y_tile + dy, tile_screen_x + dx * TILE_SIZE, tile_screen_y + dy * TILE_SIZE);
    }
  }
  LOG("Map render time: ", millis() - startMillis, " ms");
  drawMyMarker();
  drawTargetMarker();
  drawRoute();
}

void showAddresses() {
  int id = 0;
  for (const auto res : foundAddrs) {
    ui.findButtonById(id++).caption(res.name).visible(true);
  }
  ui.update();
}

void toggleKeyboard() {
  isKeyboardActive = !isKeyboardActive;
  if (!isKeyboardActive) {
    drawMap();
  } else {
    ui.findInputById('a').clear();
    ui.toggleBtnByType('a', false);
  }
  ui.findInputById('a').visible(isKeyboardActive);
  ui.toggleBtnByType('k', isKeyboardActive);
  ui.update();
}

void onAddressPressed(const Button& btn) {
  centerLoc = foundAddrs[btn.id].location;
  targetLoc = centerLoc;
  drawMap();
  for (int i = 0; i < ADDR_SEARCH_LIMIT; i++) ui.findButtonById(i).caption("").visible(false);
  ui.update();
}

void onZoomBtnPressed(const Button& btnZoom) {
  ui.update();
  zoom += btnZoom.text == "+" ? 1 : -1;
  ui.findButtonByText("+").enabled(zoom < 18);
  ui.findButtonByText("-").enabled(zoom > 12);
  drawMap();
  ui.update();
}

void onAddrBtnPressed(Button& btnAddr) {
  btnAddr._pressed = isKeyboardActive;
  btnAddr.updateAfterMs = 0;
  toggleKeyboard();
}

void onRouteBtnPressed(Button& _) {
  ui.update();
  pathFinder.findPath(myLoc.lat ? myLoc : centerLoc, targetLoc);
  drawMap();
  ui.update();
}

void searchAddress(const String& text) {
  foundAddrs.clear();
  String modifiedText = text;
  modifiedText.replace(' ', '%');
  const String query = "SELECT str, num, lon, lat, details FROM addr WHERE alias LIKE '%"
    + modifiedText
    + "%' ORDER BY CAST(num AS INTEGER) ASC LIMIT "
    + ADDR_SEARCH_LIMIT;
  const char* queryCStr = query.c_str();
  sqlite3_exec(addrDb, queryCStr,
               [](void* data, int argc, char** argv, char** azColName) -> int {
                 const String name = String(argv[0]) + " " + String(argv[1]) + " " + String(argv[4]);
                 const float lon = atof(argv[2]);
                 const float lat = atof(argv[3]);
                 foundAddrs.push_back(Address{name, {lon, lat}});
                 return 0;
               }, (void*)dbData, &zErrMsg);
  showAddresses();
}

void onAddrType(Button& btn) {
  Input& inp = ui.findInputById('a');
  if (btn.text == "<") {
    inp.removeChar();
    ui.drawInput(inp);
  } else if (btn.text == ">") {
    ui.update();
    searchAddress(inp.text);
    toggleKeyboard();
  } else {
    inp.addChar(btn.text[0]);
    ui.drawInput(inp);
  }
}

void onClick(const Pos& p) {
#ifndef DISABLE_UI
  if (!ui.processPress(p.x, p.y)) {
    targetLoc = coord::pointToLocation(p, centerLoc, zoom);
    ui.findButtonByText("R").enabled(true);
    drawMap();
    ui.update();
  }
#endif
}

void onDrag(const Pos& p) {
  Point start = coord::locationToPixels(centerLoc, zoom);
  Point end = {start.x - p.dx, start.y - p.dy};
  centerLoc = coord::pixelsToLocation(end, zoom);
  drawMap();
#ifndef DISABLE_UI
  ui.updateAfter(ANIM_MS * 2);
#endif
}

void createKeyboard() {
  constexpr char _keys[] = KEYBOARD;
  int idx = 0;
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 10; x++) {
      if (idx == 40) break;
      ui.addButton(_keys[idx++], x * (BUTTON_W + BUTTON_SPACING) + KEYBOARD_X, y * (BUTTON_H + BUTTON_SPACING) + KEYBOARD_Y)
        .type('k')
        .visible(false)
        .onPress(onAddrType);
    }
  }
  constexpr int addrH = SCREEN_HEIGHT / ADDR_SEARCH_LIMIT;
  for (int i = 0; i < ADDR_SEARCH_LIMIT; i++) // addresses
    ui.addButton("", 0, i * addrH,SCREEN_WIDTH, addrH, i)
      .visible(false)
      .type('a')
      .onPress(onAddressPressed);
}

int dbOpen(const char* filename, sqlite3** db) {
  rc = sqlite3_open(filename, db);
  if (rc) {
    LOG("Can't open database: %s\n", sqlite3_errmsg(*db));
    return rc;
  }
  return rc;
}

void setup() {
  int waitCount = 0;
  Serial.begin(115200);
  while (!Serial && waitCount++ < 50) { // 5 seconds
    delay(100);
  }
  Serial.println("--------------- started ---------------");

  btStop(); // Bluetooth OFF

#ifndef DISABLE_TFT
  LOGI("Init TFT ");
  TFT.init();
  TFT.setAttribute(UTF8_SWITCH, 1);
  TFT.setRotation(2);
  TFT.fillScreen(TFT_BLACK);
  TFT.setTextColor(TFT_BLACK);
  TFT.setFreeFont(&FreeMono9pt7b); // custom with Polish letters
  LOG("ok");
#endif

#ifndef DISABLE_UI
  LOGI("Init UI ");
  ui.init(TFT);
  ui.addButton('+', 10, 10).enabled(zoom < 18).onPress(onZoomBtnPressed);
  ui.addButton('-', 10, 10 + 45).enabled(zoom > 12).onPress(onZoomBtnPressed);
  ui.addButton('L', 10, 10 + 45 * 2).enabled(false);
  ui.addButton('A', 10, 10 + 45 * 3).onPress(onAddrBtnPressed);
  ui.addButton('R', 10, 10 + 45 * 4).enabled(false).onPress(onRouteBtnPressed);
  ui.addInput(0, KEYBOARD_Y - 40,SCREEN_WIDTH,BUTTON_H, 'a').visible(false);
  createKeyboard();
  LOG("ok");
#endif

#ifndef DISABLE_SD
  LOGI("Init Card reader");
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  int frq = 80000000;
  while (!SD.begin(SD_CS, spiSD, frq) && frq > 1000000) {
    frq -= 1000000;
    delay(100);
    LOGI(".");
  }
  LOG(" ok");
  LOGI("Init SD card");
  while (SD.cardType() == CARD_NONE) {
    LOGI(".");
    delay(100);
  }
  LOG(" ok");
#endif

#if !defined(DISABLE_SD) && !defined(DISABLE_DB)
  LOGI("Init DB ");
  sqlite3_initialize();
  if (dbOpen("/sd/addr.db", &addrDb)) return;
  if (rc != SQLITE_OK) {
    sqlite3_close(addrDb);
    LOG("fail");
  } else {
    pathFinder.init();
    LOG("ok");
  }
#endif

#if !defined(DISABLE_TOUCH) && !defined(DISABLE_COMPASS)
  Wire.begin(I2C_SDA, I2C_SCL, 0);
  delay(100);
#endif

#ifndef DISABLE_TOUCH
  LOGI("Init Touch ");
  if (!touch.init()) {
    LOG("fail");
  } else {
    touch.onClick(onClick);
    touch.onDrag(onDrag);
    LOG("ok");
  }
#endif

#ifndef DISABLE_GPS
  gpsUpdateAfterMs = now + GPS_UPDATE_PERIOD;
#endif

#ifndef DISABLE_COMPASS
  LOGI("Init Compass");
  compass.init();
  compass.enableDefault();
  compass.m_min = (LSM303::vector<int16_t>){-686, -545, -4};
  compass.m_max = (LSM303::vector<int16_t>){+331, +353, +4};
  LOG(" ok");
  compassUpdateAfterMs = now + COMPASS_UPDATE_PERIOD;
#endif

#ifndef DISABLE_TILE_CACHE
  LOGI("Init tiles cache ");
  initializeCache();
  LOG("ok");
#endif

#ifndef DISABLE_SERVER
  // WiFi.persistent(false);
  ServerSetup();
#endif


  lv_init();
  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb(my_tick);
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  lv_display_t* disp;
  /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  // disp = lv_tft_espi_create(SCREEN_HEIGHT, SCREEN_WIDTH, draw_buf, sizeof(draw_buf));
  // lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);

  /*Initialize the (dummy) input device driver*/
  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);

  lv_create_main_gui();


  LOG("---------------- Init done ----------------");

#ifndef DISABLE_TFT
  drawMap();
#ifndef DISABLE_UI
  ui.update();
#endif
#endif
}

void loop() {
  lv_timer_handler(); /* let the GUI do its work */
  lv_tick_inc(5); // tell LVGL how much time has passed
  delay(5); /* let this time pass */

  return;
  now = millis();

#ifndef DISABLE_TOUCH
  touch.update();
#endif

#ifndef DISABLE_COMPASS
  if (now > compassUpdateAfterMs) {
    compass.read();
    new_angle = compass.heading();
    if (abs(new_angle - angle) > COMPASS_ANGLE_STEP) {
      angle = new_angle;
      drawMyMarker();
    }
    compassUpdateAfterMs = now + COMPASS_UPDATE_PERIOD;
  }
#endif

#ifndef DISABLE_GPS
  bool gps = false;

  if (now > gpsUpdateAfterMs) {
    gps = true;
    // while (gpsSerial.available() > 0) {
    //   gps.encode(gpsSerial.read());
    // }
    // if (gps.location.isUpdated()) {
    //   float gpsLat = gps.location.lat();
    //   float gpsLon = gps.location.lng();
    //   if (std::abs(myLoc.lat - gpsLat) > MIN_COORD_CHANGE || (std::abs(myLoc.lon - gpsLon) > MIN_COORD_CHANGE)) {
    //     myLoc.lat = gpsLat;
    //     myLoc.lon = gpsLon;
    //     Button locBtn = ui.findButtonByText("L");
    //     if (!locBtn.enabled()) { // todo: support the lost location case
    //       locBtn.enabled(true);
    //       ui.update();
    //     }
    //   }
    // }
    // a9g.loop(gps);

    gpsUpdateAfterMs = now + GPS_UPDATE_PERIOD;
  }
#endif

#ifndef DISABLE_UI
  if (ui.updateAfterMs != 0 && now > ui.updateAfterMs) ui.update();
#endif

#ifndef DISABLE_SERVER
  ServerLoop();
#endif
}
