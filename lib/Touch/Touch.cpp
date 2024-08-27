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
  // auto samples =
  bbct.getSamples(&ti);
  // LOG("[14:Touch.cpp] samples:", samples);
  // if (samples) {
  //   for (int i = 0; i < ti.count; i++) {
  //     Serial.print("Touch ");
  //     Serial.print(i + 1);
  //     Serial.print(": ");;
  //     Serial.print("  x: ");
  //     Serial.print(ti.x[i]);
  //     Serial.print("  y: ");
  //     Serial.print(ti.y[i]);
  //     Serial.print("  size: ");
  //     Serial.println(ti.area[i]);
  //     Serial.println(' ');
  //   } // for each touch point
  // } //

  const bool touched = ti.count > 0;

  if (!touched) {
    data->state = LV_INDEV_STATE_RELEASED;
  } else {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = ti.x[0];
    data->point.y = ti.y[0];
  }
}

void Touch_init() {
  LOGI("Init Touch ");
  delay(100);
  bbct.init(I2C_SDA, I2C_SCL, -1, -1);
  LOG("ok");

  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);
}
