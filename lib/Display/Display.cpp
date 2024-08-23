#include "Display.h"
#include <globals.h>
#include <lv_init.h>
#include <TFT_eSPI.h>
#include <drivers/display/tft_espi/lv_tft_espi.h>

static auto tft = TFT_eSPI();

static void my_disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t*)px_map, w * h, true);
  tft.endWrite();

  lv_display_flush_ready(disp);
}

static void init_display() {
  size_t buffer_pixel_count = (SCREEN_WIDTH * SCREEN_HEIGHT) / 5;
  size_t buffer_size = buffer_pixel_count * sizeof(lv_color_t);

  lv_color_t* buf1 = (lv_color_t*)heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);
  // // lv_color_t* buf2 = (lv_color_t*)heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);

  // lv_display_t* disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  // lv_display_set_flush_cb(disp, my_disp_flush);
  // lv_display_set_buffers(disp, buf1, NULL, buffer_pixel_count, LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, buf1, buffer_pixel_count);
}

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
  tft.init();
  tft.initDMA();
  tft.setRotation(2);

  lv_init();

  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb(my_tick);
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  init_display();

  LOG("ok");
}
