#include "Display.h"
#include <globals.h>
#include <lv_init.h>
#include <TFT_eSPI.h>
#include <core/lv_obj.h>
#include <drivers/display/tft_espi/lv_tft_espi.cpp>

TFT_eSPI* tft_inst;
extern SemaphoreHandle_t xWireSemaphore;

#ifdef DEBUG
#define LV_USE_LOG 1
#endif
#if LV_USE_LOG != 0
static void my_print(lv_log_level_t level, const char* buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

static void flush_cb2(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  if (xSemaphoreTake(xWireSemaphore, portMAX_DELAY) == pdTRUE) {
    tft_inst->startWrite();
    tft_inst->setAddrWindow(area->x1, area->y1, w, h);
    tft_inst->pushColors((uint16_t*)px_map, w * h, true);
    tft_inst->endWrite();
    xSemaphoreGive(xWireSemaphore);
  }

  lv_display_flush_ready(disp);
}

static uint32_t my_tick() {
  return millis();
}

void Display_init() {
  LOGI("Init TFT ");

  lv_init();

#ifdef MINI_TFT
  size_t buffer_pixel_count = (SCREEN_WIDTH * SCREEN_HEIGHT) / 2;
#else
  size_t buffer_pixel_count = (SCREEN_WIDTH * SCREEN_HEIGHT) / 3;
#endif
  size_t buffer_size = buffer_pixel_count * sizeof(lv_color_t);

  auto* buf1 = static_cast<lv_color_t*>(heap_caps_malloc(buffer_size, MALLOC_CAP_DMA));
  auto* buf2 = static_cast<lv_color_t*>(heap_caps_malloc(buffer_size, MALLOC_CAP_DMA));

  auto dsp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, buf1, buf2, buffer_pixel_count);
  auto* dsc = (lv_tft_espi_t*)lv_display_get_driver_data(dsp);
  lv_display_set_flush_cb(dsp, flush_cb2);
  tft_inst = dsc->tft;
  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb(my_tick);
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  LOG("ok");
}
