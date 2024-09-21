#include "globals.h"
#include "lvgl.h"
#include "BootManager.h"

#define TRIP_MODE_TOLERANCE_M 20.0

LV_FONT_DECLARE(icons)

#define hidden(obj)          (lv_obj_has_flag((obj), LV_OBJ_FLAG_HIDDEN))
#define visible(obj)         (!lv_obj_has_flag((obj), LV_OBJ_FLAG_HIDDEN))
#define disabled(obj)        (lv_obj_has_state((obj), LV_STATE_DISABLED))
#define enabled(obj)         (!lv_obj_has_state((obj), LV_STATE_DISABLED))

#define hide(obj)            (lv_obj_add_flag((obj), LV_OBJ_FLAG_HIDDEN))
#define show(obj)            (lv_obj_remove_flag((obj), LV_OBJ_FLAG_HIDDEN))
#define disable(obj)         (lv_obj_add_state((obj), LV_STATE_DISABLED))
#define enable(obj)          (lv_obj_remove_state((obj), LV_STATE_DISABLED))

#define run_after(ms, block) \
lv_timer_create([](lv_timer_t* timer) -> void { \
lv_timer_del(timer); \
block; \
}, ms, NULL);

const char* getSymbolByTransport(const Transport tr) {
  switch (tr) {
  case TransportAll: return "9";
  case TransportBike: return ":";
  case TransportCar: return ";";
  case TransportWalk: return "<";
  default: return " ";
  }
}

Transport getTransportByIdx(int index) {
  switch (index) {
  case 0: return TransportAll;
  case 1: return TransportWalk;
  case 2: return TransportBike;
  case 3: return TransportCar;
  }
  return TransportAll;
}

lv_obj_t* createBtn(const char* label, const int32_t x, const int32_t y, const lv_event_cb_t onClick, lv_color_t color) {
  auto btn = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn, BUTTON_W, BUTTON_H);
  lv_obj_align(btn, LV_ALIGN_TOP_LEFT, x, y);
  lv_obj_t* lbl = lv_label_create(btn);
  lv_label_set_text(lbl, label);
  lv_obj_set_style_text_font(lbl, &icons, 0);
  lv_obj_set_style_text_color(lbl, color, 0);
  lv_obj_center(lbl);
  lv_obj_add_event_cb(btn, onClick, LV_EVENT_CLICKED, NULL);
  return btn;
}

void highlight(lv_event_t* e) {
  const auto obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
  lv_obj_set_style_bg_color(obj, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);
  lv_timer_create([](lv_timer_t* timer) -> void {
    auto obj = static_cast<lv_obj_t*>(lv_timer_get_user_data(timer));
    lv_timer_del(timer);
    lv_obj_set_style_bg_color(obj, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_0, 0);
  }, 200, obj);
}

lv_obj_t* createStatusIcon(const char* label, const int32_t x, const int32_t y, const lv_event_cb_t onClick) {
  const auto ico = lv_label_create(lv_scr_act());
  lv_obj_set_style_pad_hor(ico, 20, 0);
  lv_obj_set_style_pad_ver(ico, 5, 0);
  lv_label_set_text(ico, label);
  lv_obj_set_style_text_font(ico, &icons, 0);
  lv_obj_set_style_text_color(ico, lv_palette_main(LV_PALETTE_BLUE), 0);
  lv_obj_align(ico, LV_ALIGN_TOP_RIGHT, x, y);
  if (onClick) {
    lv_obj_add_flag(ico, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ico, onClick, LV_EVENT_CLICKED,NULL);
    lv_obj_add_event_cb(ico, highlight, LV_EVENT_PRESSING,NULL);
  }
  return ico;
}

// --- MATH ---
/**
 * Converts a geographical location (latitude and longitude) to pixel coordinates
 * based on the given zoom level.
 *
 * @param loc The geographical location to convert (latitude and longitude).
 * @param zoom The zoom level used for the conversion.
 * @return A lv_point_t structure containing the pixel coordinates (x, y).
 */
lv_point_t locToPx(Location loc, int zoom) {
  int n = 1 << zoom; // Number of tiles at the current zoom level
  int x = static_cast<int>((loc.lon + 180.0) / 360.0 * n * TILE_SIZE);
  float radLat = loc.lat * M_PI / 180.0; // Convert latitude to radians
  int y = static_cast<int>((1 - std::log(std::tan(radLat) + 1 / std::cos(radLat)) / M_PI) / 2 * n * TILE_SIZE);
  return {x, y}; // Return the pixel coordinates
}

/**
 * Converts a geographical location (latitude and longitude) to precise pixel coordinates
 * based on the given zoom level, using higher precision for x and y values.
 *
 * @param loc The geographical location to convert (latitude and longitude).
 * @param zoom The zoom level used for the conversion.
 * @return A lv_point_precise_t structure containing the precise pixel coordinates (x, y).
 */
lv_point_precise_t locToPPx(Location loc, int zoom) {
  int n = 1 << zoom; // Number of tiles at the current zoom level
  lv_value_precise_t x = ((loc.lon + 180.0) / 360.0 * n * TILE_SIZE);
  float radLat = loc.lat * M_PI / 180.0; // Convert latitude to radians
  lv_value_precise_t y = ((1 - std::log(std::tan(radLat) + 1 / std::cos(radLat)) / M_PI) / 2 * n * TILE_SIZE);
  return {x, y}; // Return the precise pixel coordinates
}

/**
 * Calculates the offset of a geographical location from a center location in pixel coordinates,
 * based on the given zoom level.
 *
 * @param loc The geographical location to calculate the offset for.
 * @param centerLoc The center geographical location used for the offset calculation.
 * @param zoom The zoom level used for the calculations.
 * @return A lv_point_t structure containing the offset in pixel coordinates (x, y).
 */
lv_point_t locToCenterOffsetPx(Location loc, Location centerLoc, int zoom) {
  lv_point_t loc_px = locToPx(loc, zoom); // Convert location to pixel coordinates
  lv_point_t center_px = locToPx(centerLoc, zoom); // Convert center location to pixel coordinates
  int screenX = loc_px.x - center_px.x; // Calculate the x offset
  int screenY = loc_px.y - center_px.y; // Calculate the y offset
  return {screenX, screenY}; // Return the offset in pixel coordinates
}

/**
 * Calculates the precise offset of a geographical location from a center location in pixel coordinates,
 * based on the given zoom level, using higher precision for x and y values.
 *
 * @param loc The geographical location to calculate the offset for.
 * @param centerLoc The center geographical location used for the offset calculation.
 * @param zoom The zoom level used for the calculations.
 * @return A lv_point_precise_t structure containing the precise offset in pixel coordinates (x, y).
 */
lv_point_precise_t locToCenterOffsetPPx(Location loc, Location centerLoc, int zoom) {
  lv_point_precise_t loc_px = locToPPx(loc, zoom); // Convert location to precise pixel coordinates
  lv_point_precise_t center_px = locToPPx(centerLoc, zoom); // Convert center location to precise pixel coordinates
  lv_value_precise_t screenX = SCREEN_CENTER_X + loc_px.x - center_px.x; // Calculate the precise x offset
  lv_value_precise_t screenY = SCREEN_CENTER_Y + loc_px.y - center_px.y; // Calculate the precise y offset
  return {screenX, screenY}; // Return the precise offset in pixel coordinates
}

/**
 * Calculates the haversine distance between two geographical locations (latitude and longitude),
 * which is the great-circle distance between the two points on the Earth's surface.
 *
 * @param loc1 The first geographical location.
 * @param loc2 The second geographical location.
 * @return The distance in kilometers between the two locations.
 */
float haversineDistance(Location loc1, Location loc2) {
  const float R = 6371; // Radius of the Earth in kilometers
  float dLat = radians(loc2.lat - loc1.lat); // Difference in latitude in radians
  float dLon = radians(loc2.lon - loc1.lon); // Difference in longitude in radians
  float a = sin(dLat / 2) * sin(dLat / 2) +
    cos(radians(loc1.lat)) * cos(radians(loc2.lat)) *
    sin(dLon / 2) * sin(dLon / 2); // Haversine formula
  float c = 2 * atan2(sqrt(a), sqrt(1 - a)); // Calculate the angular distance
  return R * c; // Return the distance in kilometers
}

/**
 * Converts pixel coordinates back to a geographical location (latitude and longitude)
 * based on the given zoom level.
 *
 * @param pixels The pixel coordinates to convert (x, y).
 * @param zoom The zoom level used for the conversion.
 * @return A Location structure containing the geographical coordinates (longitude, latitude).
 */
Location pxToLoc(lv_point_t pixels, int zoom) {
  int n = 1 << zoom; // Number of tiles at the current zoom level
  float lon = pixels.x / static_cast<float>(n * TILE_SIZE) * 360.0 - 180.0; // Convert pixel x to longitude
  float lat_rad = std::atan(std::sinh(M_PI * (1 - 2.0 * pixels.y / static_cast<float>(n * TILE_SIZE)))); // Calculate latitude in radians
  float lat = lat_rad * 180.0 / M_PI; // Convert radians to degrees
  return Location{lon, lat}; // Return the geographical coordinates
}

/**
 * Converts a point in pixel coordinates to a geographical location (latitude and longitude),
 * using the cursor location as a reference for the conversion.
 *
 * @param point The point in pixel coordinates (x, y) to convert.
 * @param cursorLoc The cursor geographical location used as a reference.
 * @param zoom The zoom level used for the conversion.
 * @return A Location structure containing the geographical coordinates (longitude, latitude).
 */
Location pointToLocation(lv_point_t point, Location cursorLoc, int zoom) {
  lv_point_t centerPixels = locToPx(cursorLoc, zoom); // Convert the cursor location to pixel coordinates
  int pixelX = point.x + centerPixels.x - SCREEN_CENTER_X; // Calculate the adjusted x pixel coordinate
  int pixelY = point.y + centerPixels.y - SCREEN_CENTER_Y; // Calculate the adjusted y pixel coordinate
  return pxToLoc({pixelX, pixelY}, zoom); // Convert adjusted pixel coordinates back to geographical location
}

float getDistanceMeters(Location loc1, Location loc2) {
  const float R = 6371000;
  float dLat = radians(loc2.lat - loc1.lat);
  float dLon = radians(loc2.lon - loc1.lon);
  float a = dLat * dLat + dLon * dLon * cos(radians(loc1.lat)) * cos(radians(loc2.lat));
  return R * sqrt(a);
}

float pointToLineDistance(Location p, Location p1, Location p2, Location& intersection) {
  float A = p.lat - p1.lat;
  float B = p.lon - p1.lon;
  float C = p2.lat - p1.lat;
  float D = p2.lon - p1.lon;

  float dot = A * C + B * D;
  float len_sq = C * C + D * D;
  float param = (len_sq != 0) ? dot / len_sq : -1;

  if (param < 0) {
    intersection = p1;
  } else if (param > 1) {
    intersection = p2;
  } else {
    intersection.lat = p1.lat + param * C;
    intersection.lon = p1.lon + param * D;
  }

  float dx = p.lat - intersection.lat;
  float dy = p.lon - intersection.lon;
  return sqrt(dx * dx + dy * dy) * 111320;
}

lv_point_t rotatePoint(lv_point_t point, lv_point_t center, float angle) {
  float rad = angle * M_PI / 180.0;
  float cosAngle = cos(rad);
  float sinAngle = sin(rad);

  lv_point_t rotated;
  rotated.x = cosAngle * (point.x - center.x) - sinAngle * (point.y - center.y) + center.x;
  rotated.y = sinAngle * (point.x - center.x) + cosAngle * (point.y - center.y) + center.y;
  return rotated;
}

lv_point_t locToCenterPxOffsetPx(Location loc, lv_point_t center_px, int zoom) {
  lv_point_t loc_px = locToPx(loc, zoom); // Convert location to pixel coordinates
  int screenX = loc_px.x - center_px.x; // Calculate the x offset
  int screenY = loc_px.y - center_px.y; // Calculate the y offset
  return {screenX, screenY}; // Return the offset in pixel coordinates
}

struct ClosestEdge {
  int edgeIndex;
  Location intersection;
  float distance;
};

ClosestEdge findClosestEdge(Location& my_location, const std::vector<Location>& route) {
  ClosestEdge closestEdge;
  closestEdge.distance = std::numeric_limits<float>::max();

  for (int i = 0; i < route.size() - 1; ++i) {
    Location p1 = route[i];
    Location p2 = route[i + 1];
    Location intersection;

    float distance = pointToLineDistance(my_location, p1, p2, intersection);

    if (distance < closestEdge.distance) {
      closestEdge.edgeIndex = i;
      closestEdge.intersection = intersection;
      closestEdge.distance = distance;
    }
  }

  return closestEdge;
}

ClosestEdge findClosestEdge(Location& my_location, const std::vector<RouteExt>& route) {
  ClosestEdge closestEdge;
  closestEdge.distance = std::numeric_limits<float>::max();

  for (int i = 0; i < route.size() - 1; ++i) {
    Location p1 = route[i].point;
    Location p2 = route[i + 1].point;
    Location intersection;

    float distance = pointToLineDistance(my_location, p1, p2, intersection);

    if (distance < closestEdge.distance) {
      closestEdge.edgeIndex = i;
      closestEdge.intersection = intersection;
      closestEdge.distance = distance;
    }
  }

  return closestEdge;
}

void updateRouteProgress(Location& my_location, std::vector<Location>& route) {
  ClosestEdge closestEdge = findClosestEdge(my_location, route);
  if (closestEdge.distance > TRIP_MODE_TOLERANCE_M) return;
  if (closestEdge.edgeIndex < route.size()) {
    route.erase(route.begin(), route.begin() + closestEdge.edgeIndex);
  }
  if (!route.empty()) {
    route[0] = closestEdge.intersection;
  }
}

void updateRouteExtProgress(Location& my_location, std::vector<RouteExt>& route) {
  ClosestEdge closestEdge = findClosestEdge(my_location, route);
  if (closestEdge.distance > TRIP_MODE_TOLERANCE_M) return;
  if (closestEdge.edgeIndex < route.size()) {
    route.erase(route.begin(), route.begin() + closestEdge.edgeIndex);
  }
  if (!route.empty()) {
    route[0].point = closestEdge.intersection;
    if (route.size() > 1) route[0].distance = getDistanceMeters(route[0].point, route[1].point);
  }
}

float simpleDistance(Location& loc1, Location& loc2) {
  float dLat = loc2.lat - loc1.lat;
  float dLon = (loc2.lon - loc1.lon) * cos(loc1.lat * M_PI / 180.0);
  return sqrt(dLat * dLat + dLon * dLon) * 111.32;
}

int calculateAngle(Location p1, Location p2, Location p3) {
  float dLon1 = p2.lon - p1.lon;
  float dLat1 = p2.lat - p1.lat;
  float dLon2 = p3.lon - p2.lon;
  float dLat2 = p3.lat - p2.lat;

  float angle1 = atan2(dLat1, dLon1);
  float angle2 = atan2(dLat2, dLon2);

  float angle = angle2 - angle1;

  angle = angle * 180.0 / M_PI;

  if (angle > 180) {
    angle -= 360;
  } else if (angle < -180) {
    angle += 360;
  }

  return static_cast<int>(-angle);
}
