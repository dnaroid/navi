#include "Touch.h"
#include <globals.h>
#include <Wire.h>
#include <indev/lv_indev.h>
#include <misc/lv_types.h>
#include <bb_captouch.h>

BBCapTouch bbct;
int i;
TOUCHINFO ti;

static void my_touchpad_read(lv_indev_t* indev, lv_indev_data_t* data) {
  bbct.getSamples(&ti);
  if (ti.count > 0) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = ti.x[0];
    data->point.y = ti.y[0];
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
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
