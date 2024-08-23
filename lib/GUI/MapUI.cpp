#include "MapUI.h"
#include "lvgl.h"
#include <globals.h>
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TILE_SIZE 256
#define TILES_X_SCAN {0,-1,1}
#define TILES_Y_SCAN {0,-1,1}
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
    int x;
    int y;
    int tile_x;
    int tile_y;
    int tile_ox; // offset from central tile
    int tile_oy;
    int zoom;
};

struct Marker {
    lv_obj_t* obj;
    Location loc;
    int x;
    int y;
};

static Location centerLoc;
static int zoom = 0;

static lv_obj_t* map_bg;
static lv_obj_t* btn_zoom_in;
static lv_obj_t* btn_zoom_out;
static lv_obj_t* btn_gps;

static Marker marker_me;
static Marker marker_target;

static std::vector<Tile> tiles;
static std::vector<Marker> markers;

static lv_point_t locToPx(Location loc, int zoom) {
    int n = 1 << zoom;
    int x = static_cast<int>((loc.lon + 180.0) / 360.0 * n * TILE_SIZE);
    float radLat = loc.lat * M_PI / 180.0;
    int y = static_cast<int>((1 - std::log(std::tan(radLat) + 1 / std::cos(radLat)) / M_PI) / 2 * n * TILE_SIZE);
    return lv_point_t{x, y};
}

static lv_point_t locToCenterOffsetPx(Location loc, Location centerLoc, int zoom) {
    lv_point_t loc_px = locToPx(loc, zoom);
    lv_point_t center_px = locToPx(centerLoc, zoom);
    int screenX = loc_px.x - center_px.x;
    int screenY = loc_px.y - center_px.y;
    return {screenX, screenY};
}

static Location pxToLoc(lv_point_t pixels, int zoom) {
    int n = 1 << zoom;
    float lon = pixels.x / static_cast<float>(n * TILE_SIZE) * 360.0 - 180.0;
    float lat_rad = std::atan(std::sinh(M_PI * (1 - 2.0 * pixels.y / static_cast<float>(n * TILE_SIZE))));
    float lat = lat_rad * 180.0 / M_PI;
    return Location{lon, lat};
}

static Location pointToLocation(lv_point_t point, Location cursorLoc, int zoom) {
    lv_point_t centerPixels = locToPx(cursorLoc, zoom);
    int pixelX = point.x + centerPixels.x - SCREEN_CENTER_X;
    int pixelY = point.y + centerPixels.y - SCREEN_CENTER_Y;
    return pxToLoc({pixelX, pixelY}, zoom);
}

static void update_markers() {
    for (auto& m : markers) {
        LOG("loc", m.loc.lon, m.loc.lat);
        auto c = locToCenterOffsetPx(m.loc, centerLoc, zoom);
        LOG("prev xy", m.x, m.y);
        m.x = c.x;
        m.y = c.y;
        lv_obj_set_pos(m.obj, m.x, m.y);
        LOG("new  xy", m.x, m.y);
    }
}

static void update_map() {
    lv_point_t centerPx = locToPx(centerLoc, zoom);
    int tile_x = centerPx.x / TILE_SIZE;
    int tile_y = centerPx.y / TILE_SIZE;
    int x_offset = centerPx.x % TILE_SIZE;
    int y_offset = centerPx.y % TILE_SIZE;
    bool changedTile = tiles[0].tile_x != tile_x || tiles[0].tile_y != tile_y || tiles[0].zoom != zoom;

    for (auto& t : tiles) {
        t.x = SCREEN_CENTER_X + t.tile_ox * TILE_SIZE - x_offset;
        t.y = SCREEN_CENTER_Y + t.tile_oy * TILE_SIZE - y_offset;
        if (changedTile) {
            t.tile_x = tile_x + t.tile_ox;
            t.tile_y = tile_y + t.tile_oy;
            t.zoom = zoom;
            char filename[FILE_NAME_SIZE];
            sprintf(filename, "S:/tiles/%u/%u/%u.png", zoom, t.tile_x, t.tile_y);
            lv_img_set_src(t.obj, filename);
        }
        lv_obj_set_pos(t.obj, t.x, t.y);
    }
}

static void update_buttons() {
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

static void onDrag(lv_event_t* e) {
    const lv_indev_t* indev = lv_indev_get_act();
    if (indev == NULL) return;
    lv_point_t delta;
    lv_indev_get_vect(indev, &delta);

    if (abs(delta.x) <= DRAG_THRESHOLD && abs(delta.y) <= DRAG_THRESHOLD) { return; }

    lv_point_t centerPx = locToPx(centerLoc, zoom);
    lv_point_t newCenterPx = {centerPx.x - delta.x, centerPx.y - delta.y};
    centerLoc = pxToLoc(newCenterPx, zoom);

    update_map();
    update_markers();
}

static void onZoom(lv_event_t* e) {
    void* btn = lv_event_get_target(e);
    int zoom_change = (btn == btn_zoom_in) ? 1 : -1;
    int new_zoom = zoom + zoom_change;
    if (new_zoom == zoom) return;
    zoom = new_zoom;
    update_map();
    update_markers();
    update_buttons();
}

static void onClickTile(lv_event_t* e) {
    lv_indev_t* indev = lv_indev_get_act();
    if (indev == nullptr) return;
    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);
    if (abs(vect.x) > DRAG_THRESHOLD || abs(vect.y) > DRAG_THRESHOLD) { return; }
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    for (auto& t : tiles) {
        if (t.obj == obj) {
            lv_point_t point;
            lv_indev_get_point(lv_indev_get_act(), &point);
            auto new_location = pointToLocation(point, centerLoc, zoom);
            lv_obj_clear_flag(marker_target.obj, LV_OBJ_FLAG_HIDDEN);
            LOG("prev loc", marker_target.loc.lon, marker_target.loc.lat);
            marker_target.loc = new_location;
            LOG("new  loc", marker_target.loc.lon, marker_target.loc.lat);

            update_markers();
            break;
        }
    }
}

static void onGps(lv_event_t* e) {
    centerLoc = {18.620855, 54.393417};
    zoom = 16;
    update_map();
    update_markers();
    update_buttons();
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

static void create_map() {
    map_bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(map_bg, lv_pct(100), lv_pct(100));
    lv_obj_align(map_bg, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(map_bg, LV_OBJ_FLAG_SCROLLABLE);

    lv_point_t centerPx = locToPx(centerLoc, zoom);
    int tile_x = centerPx.x / TILE_SIZE;
    int tile_y = centerPx.y / TILE_SIZE;
    int x_offset = centerPx.x % TILE_SIZE;
    int y_offset = centerPx.y % TILE_SIZE;

    for (const int dx : TILES_X_SCAN) {
        for (const int dy : TILES_Y_SCAN) {
            const Tile t = {
                lv_img_create(map_bg),
                SCREEN_CENTER_X + dx * TILE_SIZE - x_offset,
                SCREEN_CENTER_Y + dy * TILE_SIZE - y_offset,
                tile_x + dx,
                tile_y + dy,
                dx,
                dy,
                0
            };
            /*static lv_style_t style;
            lv_style_init(&style);
            lv_style_set_border_width(&style, 1);
            lv_style_set_border_color(&style, lv_color_hex(0xFF0000));
            lv_obj_add_style(new_tile.obj, &style, LV_PART_MAIN);*/
            lv_obj_align(t.obj, LV_ALIGN_TOP_LEFT, 0, 0);
            lv_obj_add_flag(t.obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(t.obj, onDrag, LV_EVENT_PRESSING, NULL);
            lv_obj_add_event_cb(t.obj, onClickTile, LV_EVENT_CLICKED, NULL);

            tiles.push_back(t);
        }
    }
    update_map();
}

static void create_buttons() {
    int x = 2, y = 10, step = 50;
    btn_zoom_in = create_btn(LV_SYMBOL_PLUS, x, y, onZoom);
    y += step;
    btn_zoom_out = create_btn(LV_SYMBOL_MINUS, x, y, onZoom);
    y += step;
    btn_gps = create_btn(LV_SYMBOL_GPS, x, y, onGps);

    update_buttons();
}

static void create_markers() {
    auto c = locToCenterOffsetPx(centerLoc, centerLoc, zoom);

    auto trg = lv_label_create(map_bg);
    lv_label_set_text(trg, LV_SYMBOL_DOWNLOAD);
    lv_obj_set_style_text_color(trg, lv_color_hex(0x0000FF), 0);
    lv_obj_align(trg, LV_ALIGN_CENTER, 0, 0);
    marker_target = {trg, {0, 0}, 0, 0};
    lv_obj_add_flag(trg, LV_OBJ_FLAG_HIDDEN);
    markers.push_back(marker_target);

    // auto me = lv_label_create(map_bg);
    // lv_label_set_text(me, LV_SYMBOL_PLAY);
    // lv_obj_set_style_transform_angle(me, -900, 0);
    // lv_obj_set_style_text_color(me, lv_color_hex(0x0000FF), 0);
    // lv_obj_align(me, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_pos(me, c.x, c.y);
    // marker_me = {me, centerLoc, c.x, c.y};
    // markers.push_back(marker_me);

    update_markers();
}

void Map_init(Location center, int initZoom) {
    LOGI("Init Map");
    centerLoc = center;
    zoom = initZoom;

    create_map();
    create_markers();
    create_buttons();

    LOG(" ok");
}

void Map_destroy() {
    lv_obj_del(map_bg);
    lv_obj_del(marker_me.obj);
    for (const auto& tile : tiles) {
        lv_obj_del(tile.obj);
    }
}
