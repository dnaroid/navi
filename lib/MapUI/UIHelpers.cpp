#include "globals.h"
#include "lvgl.h"
LV_FONT_DECLARE(icons)

// --- syntax sugar ---

bool isHidden(const lv_obj_t* obj) {
  return lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN);
}

bool isVisible(const lv_obj_t* obj) {
  return !lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN);
}

void hide(lv_obj_t* obj) {
  lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
}

void show(lv_obj_t* obj) {
  lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN);
}

bool isDisabled(const lv_obj_t* obj) {
  return lv_obj_has_state(obj, LV_STATE_DISABLED);
}

void disable(lv_obj_t* obj) {
  lv_obj_add_state(obj, LV_STATE_DISABLED);
}

void enable(lv_obj_t* obj) {
  lv_obj_clear_state(obj, LV_STATE_DISABLED);
}

lv_obj_t* createBtn(const char* label, const int32_t x, const int32_t y, const lv_event_cb_t onClick, const int32_t w = 40, const int32_t h = 40) {
  lv_obj_t* btn = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn, w, h);
  lv_obj_align(btn, LV_ALIGN_TOP_LEFT, x, y);
  lv_obj_t* lbl = lv_label_create(btn);
  lv_label_set_text(lbl, label);
  lv_obj_set_style_text_font(lbl, &icons, 0);
  lv_obj_center(lbl);
  lv_obj_add_event_cb(btn, onClick, LV_EVENT_CLICKED, NULL);
  return btn;
}

// --- MATH ---

lv_point_t locToPx(Location loc, int zoom) {
  int n = 1 << zoom;
  int x = static_cast<int>((loc.lon + 180.0) / 360.0 * n * TILE_SIZE);
  float radLat = loc.lat * M_PI / 180.0;
  int y = static_cast<int>((1 - std::log(std::tan(radLat) + 1 / std::cos(radLat)) / M_PI) / 2 * n * TILE_SIZE);
  return {x, y};
}

lv_point_precise_t locToPPx(Location loc, int zoom) {
  int n = 1 << zoom;
  lv_value_precise_t x = ((loc.lon + 180.0) / 360.0 * n * TILE_SIZE);
  float radLat = loc.lat * M_PI / 180.0;
  lv_value_precise_t y = ((1 - std::log(std::tan(radLat) + 1 / std::cos(radLat)) / M_PI) / 2 * n * TILE_SIZE);
  return {x, y};
}

lv_point_t locToCenterOffsetPx(Location loc, Location centerLoc, int zoom) {
  lv_point_t loc_px = locToPx(loc, zoom);
  lv_point_t center_px = locToPx(centerLoc, zoom);
  int screenX = loc_px.x - center_px.x;
  int screenY = loc_px.y - center_px.y;
  return {screenX, screenY};
}

lv_point_precise_t locToCenterOffsetPPx(Location loc, Location centerLoc, int zoom) {
  lv_point_precise_t loc_px = locToPPx(loc, zoom);
  lv_point_precise_t center_px = locToPPx(centerLoc, zoom);
  lv_value_precise_t screenX = SCREEN_CENTER_X + loc_px.x - center_px.x;
  lv_value_precise_t screenY = SCREEN_CENTER_Y + loc_px.y - center_px.y;
  return {screenX, screenY};
}

double haversineDistance(Location loc1, Location loc2) {
  const double R = 6371;
  double dLat = radians(loc2.lat - loc1.lat);
  double dLon = radians(loc2.lon - loc1.lon);
  double a = sin(dLat / 2) * sin(dLat / 2) +
    cos(radians(loc1.lat)) * cos(radians(loc2.lat)) *
    sin(dLon / 2) * sin(dLon / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return R * c;
}

Location pxToLoc(lv_point_t pixels, int zoom) {
  int n = 1 << zoom;
  float lon = pixels.x / static_cast<float>(n * TILE_SIZE) * 360.0 - 180.0;
  float lat_rad = std::atan(std::sinh(M_PI * (1 - 2.0 * pixels.y / static_cast<float>(n * TILE_SIZE))));
  float lat = lat_rad * 180.0 / M_PI;
  return Location{lon, lat};
}

Location pointToLocation(lv_point_t point, Location cursorLoc, int zoom) {
  lv_point_t centerPixels = locToPx(cursorLoc, zoom);
  int pixelX = point.x + centerPixels.x - SCREEN_CENTER_X;
  int pixelY = point.y + centerPixels.y - SCREEN_CENTER_Y;
  return pxToLoc({pixelX, pixelY}, zoom);
}

