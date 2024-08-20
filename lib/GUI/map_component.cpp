#include "map_component.h"
#include <SD.h>
#include <globals.h>


int longitudeToPixels(float lon, int zoom) {
    return static_cast<int>((lon + 180.0) / 360.0 * (1 << zoom) * TILE_SIZE);
}

int latitudeToPixels(float lat, int zoom) {
    float radLat = lat * M_PI / 180.0;
    return static_cast<int>((1.0 - std::log(std::tan(radLat) + 1.0 / std::cos(radLat)) / M_PI) / 2.0 * (1 << zoom) * TILE_SIZE);
}

int longitudeToTileX(float lon, int zoom) {
    return static_cast<int>((lon + 180.0f) / 360.0f * (1 << zoom));
}

int latitudeToTileY(float lat, int zoom) {
    float radLat = lat * M_PI / 180.0f;
    return static_cast<int>((1.0f - std::log(std::tan(radLat) + 1.0f / std::cos(radLat)) / M_PI) / 2.0f * (1 << zoom));
}

int lon_to_tile_x(float lon, int zoom) {
    return static_cast<int>(floor((lon + 180.0) / 360.0 * (1 << zoom)));
}

int lat_to_tile_y(float lat, int zoom) {
    float latrad = lat * M_PI / 180.0;
    return static_cast<int>(floor((1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << zoom)));
}

char filename_lv[60] = {};

lv_obj_t* main_bg_img_M = NULL;

MapComponent::MapComponent(lv_obj_t* parent, float lat, float lon, int zoom) {
    center_lat = lat;
    center_lon = lon;
    this->zoom = zoom;

    map_container = lv_obj_create(lv_screen_active());
    // lv_obj_set_size(map_container, lv_pct(100), lv_pct(100));
    lv_obj_set_size(map_container, TILE_SIZE * 3, TILE_SIZE * 3);
    // lv_obj_clear_flag(map_container, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_scroll_dir(map_container, LV_DIR_HOR | LV_DIR_VER);
    // lv_obj_set_scroll_snap_x(map_container, LV_SCROLL_SNAP_NONE);
    // lv_obj_set_scroll_snap_y(map_container, LV_SCROLL_SNAP_NONE);

    // Обработка событий прокрутки
    // lv_obj_add_event_cb(map_container, [](lv_event_t* e) {
    //     auto* component = static_cast<MapComponent*>(lv_event_get_user_data(e));
    //     component->update_visible_tiles();
    // }, LV_EVENT_SCROLL, this);

    int x_tile = lon_to_tile_x(center_lon, zoom);
    int y_tile = lat_to_tile_y(center_lat, zoom);
    main_bg_img_M = lv_img_create(map_container);
    filename_lv[50] = {};
    sprintf(filename_lv, "S:/tiles/%u/%u/%u.png", zoom, x_tile, y_tile);
    LOG(filename_lv);
    lv_img_set_src(main_bg_img_M, filename_lv);
    // lv_obj_align(main_bg_img_M, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_size(main_bg_img_M, TILE_SIZE, TILE_SIZE);
}

MapComponent::~MapComponent() {
    lv_obj_del(map_container);
}


