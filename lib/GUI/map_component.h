#ifndef MAP_COMPONENT_H
#define MAP_COMPONENT_H

#include <lvgl.h>

class MapComponent {
public:
  MapComponent(lv_obj_t* parent);
  ~MapComponent();

  void set_tile_size(uint16_t size);
  void set_map_path(const char* path);
  void set_initial_position(int16_t x, int16_t y);

  // Функция для обновления видимой области и загрузки тайлов
  void update_visible_tiles();

private:
  lv_obj_t* map_container;
  uint16_t tile_size;
  const char* map_path;

  // Вспомогательная функция для загрузки тайлов с SD-карты
  lv_img_dsc_t load_tile(uint16_t x, uint16_t y);

  // Функция для очистки неиспользуемых тайлов
  void clear_unused_tiles();
};

#endif // MAP_COMPONENT_H
