#ifndef MAP_COMPONENT_H
#define MAP_COMPONENT_H

#include "lvgl.h"

class MapUI {
public:
  MapUI(lv_obj_t* parent, float lat, float lon, int zoom);
  ~MapUI();

private:
  void update_map();
  void zoom_in();
  void zoom_out();

  lv_obj_t* map_container;
  lv_obj_t* main_bg_img_M;
  lv_obj_t* zoom_in_btn;
  lv_obj_t* zoom_out_btn;

  float center_lat;
  float center_lon;
  int zoom;
};

#endif // MAP_COMPONENT_H
