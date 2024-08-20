#include "Display.h"

#include <globals.h>
#include <lv_init.h>
#include <drivers/display/tft_espi/lv_tft_espi.h>

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t buf1[DRAW_BUF_SIZE];
// uint32_t buf2[DRAW_BUF_SIZE];

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char* buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

static uint32_t my_tick() {
  return millis();
}

void Display::init() {
  LOGI("Init TFT ");
  TFT.init();
  TFT.initDMA();
  TFT.setRotation(0);

  lv_init();

  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb(my_tick);
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  // lv_display_t* disp;
  // disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  // lv_display_set_flush_cb(disp, my_disp_flush);
  // lv_display_set_buffers(disp, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, buf1, sizeof(buf1));

  LOG("ok");
}
