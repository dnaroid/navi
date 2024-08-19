#include "map_component.h"
#include <SD.h>

#include <cmath>
#include <globals.h>

uint32_t lat_lon_to_tile_x(double lon, uint8_t zoom) {
    return static_cast<uint32_t>((lon + 180.0) / 360.0 * (1 << zoom));
}

uint32_t lat_lon_to_tile_y(double lat, uint8_t zoom) {
    double lat_rad = lat * M_PI / 180.0;
    return static_cast<uint32_t>((1.0 - std::log(std::tan(lat_rad) + 1.0 / std::cos(lat_rad)) / M_PI) / 2.0 * (1 << zoom));
}


MapComponent::MapComponent(lv_obj_t* parent) {
    // Инициализация параметров
    tile_size = 256; // Размер тайлов по умолчанию
    map_path = "/"; // Путь к папке с тайлами

    // Создаем контейнер карты
    map_container = lv_obj_create(parent);
    // lv_obj_set_size(map_container, lv_pct(100), lv_pct(100));
    lv_obj_set_size(map_container, tile_size * 3, tile_size * 3);
    lv_obj_set_scroll_dir(map_container, LV_DIR_HOR | LV_DIR_VER);
    lv_obj_set_scroll_snap_x(map_container, LV_SCROLL_SNAP_NONE);
    lv_obj_set_scroll_snap_y(map_container, LV_SCROLL_SNAP_NONE);

    // Обработка событий прокрутки
    lv_obj_add_event_cb(map_container, [](lv_event_t* e) {
        auto* component = static_cast<MapComponent*>(lv_event_get_user_data(e));
        component->update_visible_tiles();
    }, LV_EVENT_SCROLL, this);
}

MapComponent::~MapComponent() {
    lv_obj_del(map_container);
}

void MapComponent::set_initial_tile_position(int tile_x, int tile_y) {
    // Вычислите позицию прокрутки, чтобы тайл оказался в центре
    int16_t scroll_x = tile_x * tile_size - (lv_obj_get_width(map_container) / 2);
    int16_t scroll_y = tile_y * tile_size - (lv_obj_get_height(map_container) / 2);

    // Прокрутите к позиции
    lv_obj_scroll_to_x(map_container, scroll_x, LV_ANIM_OFF);
    lv_obj_scroll_to_y(map_container, scroll_y, LV_ANIM_OFF);

    // Загрузите видимые тайлы
    update_visible_tiles();
}

void MapComponent::set_initial_geographic_position(double lat, double lon, uint8_t zoom) {
    uint32_t tile_x = lat_lon_to_tile_x(lon, zoom);
    uint32_t tile_y = lat_lon_to_tile_y(lat, zoom);

    // Предположим, что размер тайлов равен tile_size
    int16_t scroll_x = tile_x * tile_size - (lv_obj_get_width(map_container) / 2);
    int16_t scroll_y = tile_y * tile_size - (lv_obj_get_height(map_container) / 2);

    lv_obj_scroll_to_x(map_container, scroll_x, LV_ANIM_OFF);
    lv_obj_scroll_to_y(map_container, scroll_y, LV_ANIM_OFF);

    update_visible_tiles();
}

void MapComponent::set_tile_size(uint16_t size) {
    tile_size = size;
}

void MapComponent::set_map_path(const char* path) {
    map_path = path;
}

lv_img_dsc_t MapComponent::load_tile(uint16_t x, uint16_t y) {
    static lv_img_dsc_t img_dsc;
    String filename = String(map_path) + "/18/" + String(x) + "/" + String(y) + ".png";
    File file = SD.open(filename.c_str());
    if (!file) {
        Serial.println("Failed to open file " + filename);
        return img_dsc;
    }

    // Чтение файла в буфер
    uint32_t fileSize = file.size();
    auto* img_data = (uint8_t*)malloc(fileSize);
    file.read(img_data, fileSize);
    file.close();

    img_dsc.data = img_data;
    img_dsc.data_size = fileSize;
    // img_dsc.header.always_zero = 0;
    img_dsc.header.w = tile_size;
    img_dsc.header.h = tile_size;
    // img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;

    return img_dsc;
}

void MapComponent::update_visible_tiles() {
    lv_area_t visible_area;
    lv_obj_get_coords(map_container, &visible_area);

    auto start_col = visible_area.x1 / tile_size;
    auto end_col = visible_area.x2 / tile_size;
    auto start_row = visible_area.y1 / tile_size;
    auto end_row = visible_area.y2 / tile_size;

    // Ограничьте тайлы до 3x3
    start_col = max(0, start_col);
    end_col = min(start_col + 2, start_col + 2);
    start_row = max(0, start_row);
    end_row = min(start_row + 2, start_row + 2);

    for (uint16_t x = start_col; x <= end_col; x++) {
        for (uint16_t y = start_row; y <= end_row; y++) {
            lv_img_dsc_t img_dsc = load_tile(x, y);
            if (img_dsc.data != NULL) {
                lv_obj_t* img = lv_img_create(map_container);
                lv_img_set_src(img, &img_dsc);
                lv_obj_set_pos(img, x * tile_size, y * tile_size);
            }
        }
    }
}

void MapComponent::clear_unused_tiles() {
    lv_obj_t* child = lv_obj_get_child(map_container, 0);
    while (child != NULL) {
        lv_area_t child_coords;
        lv_obj_get_coords(child, &child_coords);

        lv_area_t parent_coords;
        lv_obj_get_coords(map_container, &parent_coords);

        if (!_lv_area_is_in(&parent_coords, &child_coords, 0)) {
            lv_obj_del(child);
        }

        child = lv_obj_get_child(map_container, -1);
    }
}
