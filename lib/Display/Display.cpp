#include "Display.h"
#include <globals.h>
#include <lv_init.h>
#include <drivers/display/tft_espi/lv_tft_espi.h>
#include <drivers/display/tft_espi/lv_tft_espi.cpp>

TFT_eSPI* tft_inst;

#if LV_USE_LOG != 0
static void my_print(lv_log_level_t level, const char* buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

static uint32_t my_tick() {
  return millis();
}

void Display_init() {
  LOGI("Init TFT ");

  lv_init();

  size_t buffer_pixel_count = (SCREEN_WIDTH * SCREEN_HEIGHT) / 5;
  size_t buffer_size = buffer_pixel_count * sizeof(lv_color_t);

  auto* buf1 = static_cast<lv_color_t*>(heap_caps_malloc(buffer_size, MALLOC_CAP_DMA));

  auto dsp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, buf1, buffer_pixel_count);
  auto* dsc = (lv_tft_espi_t*)lv_display_get_driver_data(dsp);
  tft_inst = dsc->tft;


  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb(my_tick);
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  LOG("ok");
}
