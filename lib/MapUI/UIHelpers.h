#include <Mirror.h>
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

const char* getSymbolByTransport(const Transport tr);

Transport getTransportByIdx(int index);

lv_obj_t* createBtn(const char* label, const int32_t x, const int32_t y, const lv_event_cb_t onClick, lv_color_t color = lv_color_white());

void highlight(lv_event_t* e);

lv_obj_t* createStatusIcon(const char* label, const int32_t x, const int32_t y, const lv_event_cb_t onClick = nullptr);

lv_point_t locToPx(Location loc, int zoom);

lv_point_precise_t locToPPx(Location loc, int zoom);

lv_point_t locToCenterOffsetPx(Location loc, Location centerLoc, int zoom);

lv_point_precise_t locToCenterOffsetPPx(Location loc, Location centerLoc, int zoom);

float haversineDistance(Location loc1, Location loc2);

Location pxToLoc(lv_point_t pixels, int zoom);

Location pointToLocation(lv_point_t point, Location cursorLoc, int zoom);

float getDistanceMeters(Location loc1, Location loc2);

void updateRouteProgress(Location& my_location, std::vector<Location>& route);
void updateRouteExtProgress(Location& my_location, std::vector<RouteExt>& route);

lv_point_t rotatePoint(lv_point_t point, lv_point_t center, float angle);

lv_point_t locToCenterPxOffsetPx(Location loc, lv_point_t center_px, int zoom);

float simpleDistance(Location& loc1, Location& loc2);

int calculateAngle(Location p1, Location p2, Location p3);
