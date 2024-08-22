#include "MapUI.h"
#include "lvgl.h"
#include <globals.h>
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TILE_SIZE 256
#define TILES_X_SCAN {-1,0,1}
#define TILES_Y_SCAN {-1,0,1}
// #define TILES_X_SCAN {0,1,-1,2,-2}
// #define TILES_Y_SCAN {0,1,-1,2,-2}
#define MARKERS_TO_RENDER 1
#define ZOOM_MIN 12
#define ZOOM_MAX 18
#define DRAG_THRESHOLD 10
#define FILE_NAME_SIZE 40
#define MARKER_SIZE 30

struct Tile {
    lv_obj_t* obj;
    int screen_x;
    int screen_y;
    int screen_dx;
    int screen_dy;
    int tile_x;
    int tile_y;
};

struct Marker {
    lv_obj_t* obj;
    int tile_x;
    int tile_y;
    int in_tile_x;
    int in_tile_y;
};

static float center_lat = 0.0;
static float center_lon = 0.0;
static int zoom = 0;

static lv_obj_t* map_bg;
static lv_obj_t* btn_zoom_in;
static lv_obj_t* btn_zoom_out;
static std::vector<Tile> tiles;
static Marker marker_me;

static Point locationToPixels(Location loc, int zoom) {
    int n = 1 << zoom;
    int x = static_cast<int>((loc.lon + 180.0) / 360.0 * n * TILE_SIZE);
    float radLat = loc.lat * M_PI / 180.0;
    int y = static_cast<int>((1 - std::log(std::tan(radLat) + 1 / std::cos(radLat)) / M_PI) / 2 * n * TILE_SIZE);
    return Point{x, y};
}

static Marker create_marker(Location loc) {
    Point p = locationToPixels(loc, zoom);
    int tile_x = p.x / TILE_SIZE;
    int tile_y = p.y / TILE_SIZE;
    int tile_offset_x = p.x % TILE_SIZE;
    int tile_offset_y = p.y % TILE_SIZE;
    return Marker{NULL, tile_x, tile_y, tile_offset_x, tile_offset_y};
}

static void update_marker_position(Marker& marker, Tile& new_tile) {
    lv_obj_set_parent(marker.obj, new_tile.obj);
    lv_obj_set_pos(marker.obj, marker.in_tile_x, marker.in_tile_y);
}

static void update_markers() {
    for (auto& tile : tiles) {
        if (tile.tile_x == marker_me.tile_x && tile.tile_y == marker_me.tile_y) {
            if (marker_me.obj == NULL) {
                lv_obj_t* m_obj = lv_img_create(tile.obj);
                marker_me.obj = m_obj;
                lv_img_set_src(m_obj, "S:/assets/marker.png");
                lv_obj_set_size(m_obj, MARKER_SIZE, MARKER_SIZE);
                lv_obj_set_pos(m_obj, marker_me.in_tile_x, marker_me.in_tile_y);
            } else {
                if (lv_obj_get_parent(marker_me.obj) != tile.obj) {
                    update_marker_position(marker_me, tile);
                    break;
                }
            }
        }
    }
}

static void drag_event_handler(lv_event_t* e) {
    const lv_indev_t* indev = lv_indev_get_act();
    if (indev == NULL) return;
    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);
    if (abs(vect.x) <= DRAG_THRESHOLD && abs(vect.y) <= DRAG_THRESHOLD) { return; }
    for (auto& t : tiles) {
        t.screen_dx += vect.x;
        t.screen_dy += vect.y;

        bool changed = false;

        if (t.screen_dx <= -TILE_SIZE / 2) {
            t.screen_dx += TILE_SIZE;
            t.tile_x++;
            changed = true;
        } else if (t.screen_dx >= TILE_SIZE / 2) {
            t.screen_dx -= TILE_SIZE;
            t.tile_x--;
            changed = true;
        }

        if (t.screen_dy <= -TILE_SIZE / 2) {
            t.screen_dy += TILE_SIZE;
            t.tile_y++;
            changed = true;
        } else if (t.screen_dy >= TILE_SIZE / 2) {
            t.screen_dy -= TILE_SIZE;
            t.tile_y--;
            changed = true;
        }

        if (changed) {
            char filename[FILE_NAME_SIZE];
            sprintf(filename, "S:/tiles/%u/%u/%u.png", zoom, t.tile_x, t.tile_y);
            lv_img_set_src(t.obj, filename);
        }

        lv_obj_set_pos(t.obj, t.screen_x + t.screen_dx, t.screen_y + t.screen_dy);
    }
    update_markers();
}

static void update_zoom_buttons() {
    if (zoom <= ZOOM_MIN) {
        lv_obj_add_state(btn_zoom_out, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(btn_zoom_out, LV_STATE_DISABLED);
    }

    if (zoom >= ZOOM_MAX) {
        lv_obj_add_state(btn_zoom_in, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(btn_zoom_in, LV_STATE_DISABLED);
    }
}

static void update_tiles() {
    for (auto& tile : tiles) {
        char filename[FILE_NAME_SIZE];
        sprintf(filename, "S:/tiles/%u/%u/%u.png", zoom, tile.tile_x, tile.tile_y);
        lv_img_set_src(tile.obj, filename);
        lv_obj_set_pos(tile.obj, tile.screen_x + tile.screen_dx, tile.screen_y + tile.screen_dy);
    }
}

static void zoom_event_handler(lv_event_t* e) {
    void* btn = lv_event_get_target(e);
    int zoom_change = (btn == btn_zoom_in) ? 1 : -1;
    int new_zoom = zoom + zoom_change;
    if (new_zoom == zoom) return;
    zoom = new_zoom;
    // need re-calc
    // center_tile_x = lon_to_tile_x(center_lon, zoom);
    // center_tile_y = lat_to_tile_y(center_lat, zoom);
    update_tiles();
    update_zoom_buttons();
}

static lv_obj_t* create_btn(const char* label, const int32_t x, const int32_t y, const lv_event_cb_t onClick, const int32_t w = 40, const int32_t h = 40) {
    lv_obj_t* btn = lv_btn_create(map_bg);
    lv_obj_set_size(btn, w, h);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, x, y);
    lv_obj_t* zoom_in_label = lv_label_create(btn);
    lv_label_set_text(zoom_in_label, label);
    lv_obj_center(zoom_in_label);
    lv_obj_add_event_cb(btn, onClick, LV_EVENT_CLICKED, NULL);
    return btn;
}

void Map_init(Location centerLoc, int initZoom) {
    LOGI("Init Map");
    center_lat = centerLoc.lat;
    center_lon = centerLoc.lon;
    zoom = initZoom;

    Point p = locationToPixels(centerLoc, zoom);
    int center_tile_x = p.x / TILE_SIZE;
    int center_tile_y = p.y / TILE_SIZE;
    int tile_offset_x = p.x % TILE_SIZE - TILE_SIZE / 2;
    int tile_offset_y = p.y % TILE_SIZE - TILE_SIZE / 2;

    map_bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(map_bg, lv_pct(100), lv_pct(100));
    lv_obj_align(map_bg, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(map_bg, LV_OBJ_FLAG_SCROLLABLE);

    for (const int dx : TILES_X_SCAN) {
        for (const int dy : TILES_Y_SCAN) {
            const Tile new_tile = {
                lv_img_create(map_bg),
                SCREEN_WIDTH / 2 - TILE_SIZE / 2 + dx * TILE_SIZE,
                SCREEN_HEIGHT / 2 - TILE_SIZE / 2 + dy * TILE_SIZE,
                -tile_offset_x,
                -tile_offset_y,
                center_tile_x + dx,
                center_tile_y + dy
            };
            lv_obj_add_flag(new_tile.obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(new_tile.obj, drag_event_handler, LV_EVENT_PRESSING, NULL);
            tiles.push_back(new_tile);
        }
    }
    update_tiles();
    marker_me = create_marker(centerLoc);
    update_markers();
    btn_zoom_in = create_btn("+", 2, 10, zoom_event_handler);
    btn_zoom_out = create_btn("-", 2, 60, zoom_event_handler);
    update_zoom_buttons();
    LOG(" ok");
}

void Map_destroy() {
    lv_obj_del(map_bg);
    lv_obj_del(marker_me.obj);
    for (const auto& tile : tiles) {
        lv_obj_del(tile.obj);
    }
}
