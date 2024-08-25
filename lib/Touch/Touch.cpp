#include "Touch.h"
#include <globals.h>
#include <NS2009.h>
#include <Wire.h>
#include <indev/lv_indev.h>
#include <misc/lv_types.h>

static void my_touchpad_read(lv_indev_t* indev, lv_indev_data_t* data) {
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

void Touch_init() {
  LOGI("Init Touch ");
  // Wire.begin(I2C_SDA, I2C_SCL, 0);
  delay(100);
  if (!initTouch()) {
    LOG("fail");
  } else {
    LOG("ok");
  }

  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);
}
