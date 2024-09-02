#include "Touch.h"
#include <globals.h>
#include <Wire.h>
#include <indev/lv_indev.h>
#include <misc/lv_types.h>
#include <bb_captouch.h>

BBCapTouch bbct;
int i;
TOUCHINFO ti;

static lv_point_t last_points[2];
static bool is_multiTouch = false;
static float last_distance = 0.0;
static int active_touches = 0;
MultiTouchData mtZoom;

float get_distance(const lv_point_t p1, const lv_point_t p2) {
  int delta_x = p2.x - p1.x;
  int delta_y = p2.y - p1.y;
  return std::sqrt(delta_x * delta_x + delta_y * delta_y);
}

static void my_touchpad_read(lv_indev_t* indev, lv_indev_data_t* data) {
  bbct.getSamples(&ti);

  active_touches = ti.count;

  if (active_touches > 0) {
    if (active_touches == 1 && !is_multiTouch) {
      data->state = LV_INDEV_STATE_PRESSED;
      last_points[0].x = ti.x[0];
      last_points[0].y = ti.y[0];
      data->point.x = last_points[0].x;
      data->point.y = last_points[0].y;
    } else if (active_touches >= 2) {
      is_multiTouch = true;
      last_points[0].x = ti.x[0];
      last_points[0].y = ti.y[0];
      last_points[1].x = ti.x[1];
      last_points[1].y = ti.y[1];

      mtZoom.center = {
        (last_points[0].x + last_points[1].x) / 2,
        (last_points[0].y + last_points[1].y) / 2
      };

      float distance = get_distance(last_points[0], last_points[1]);
      if (last_distance > 0.0) {
        mtZoom.direction = distance > last_distance ? 1 : -1;
      }
      last_distance = distance;
    }
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
    if (is_multiTouch) {
      mtZoom.ready = true;
    }
    is_multiTouch = false;
    last_distance = 0.0;
  }
}

void Touch_init() {
  LOGI("Init Touch ");
  delay(100);
  bbct.init(I2C_SDA, I2C_SCL, -1, -1);

  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);
  LOG("ok");
}
