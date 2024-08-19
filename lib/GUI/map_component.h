#ifndef MAP_COMPONENT_H
#define MAP_COMPONENT_H

#include "lvgl.h"
#include "Arduino.h"

class MapComponent {
public:
  MapComponent(lv_obj_t* parent);
  ~MapComponent();

  void set_tile_size(uint16_t size);
  void set_map_path(const char* path);
  void set_initial_tile_position(uint16_t tile_x, uint16_t tile_y);
  void set_initial_geographic_position(double lat, double lon, uint8_t zoom);
  void set_initial_tile_position(int tile_x, int tile_y);
  void update_visible_tiles();

private:
  lv_obj_t* map_container;
  uint16_t tile_size;
  String map_path;
  int center_lat, center_lon, zoom;
  void clear_unused_tiles();
  lv_img_dsc_t load_tile(uint16_t x, uint16_t y);
};

#endif // MAP_COMPONENT_H
