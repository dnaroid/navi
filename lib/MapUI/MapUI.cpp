#include "MapUI.h"
#include "lvgl.h"
#include <BootManager.h>
#include <sqlite3.h>
#include <vector>
#include "montserrat_14_pl.c"

LV_FONT_DECLARE(montserrat_14_pl)

// #define DEBUG_TILE
#define TILE_SIZE 256
#define ZOOM_MIN 12
#define ZOOM_MAX 18
#define DRAG_THRESHOLD 10
#define FILE_NAME_SIZE 40
#define MARKER_SIZE 30
// first tile should be central!
#define TILES_X_SCAN {0,-1,1}
#define TILES_Y_SCAN {0,-1,1}
#define PRIMARY_COLOR lv_color_hex(0x2196F3)
#define TOAST_COLOR lv_color_hex(0xFF5722)

struct Address {
    String name;
    Location location;
};

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
    lv_point_t pos;
};

static Location centerLoc;
static int zoom = 0;

static char* zErrMsg = nullptr;
static auto dbData = "Callback function called";

sqlite3* addrDb;
Address address;
std::vector<Address> foundAddrs; //todo make class

static lv_obj_t* map_bg;
static lv_obj_t* btn_zoom_in;
static lv_obj_t* btn_zoom_out;
static lv_obj_t* btn_gps;
static lv_obj_t* btn_route;
static lv_obj_t* btn_search;
static lv_obj_t* line_route;
static lv_obj_t* keyboard;
static lv_obj_t* search_field;
static lv_obj_t* toast;
static lv_obj_t* addrs_list;
static Marker marker_me;
static Marker marker_target;
static lv_point_precise_t* route_px;

static std::vector<Tile> tiles;
static std::vector<Marker> markers;

static std::vector<Location> route = {};
static float distance = -1;

static lv_point_t locToPx(Location loc, int zoom) {
    int n = 1 << zoom;
    int x = static_cast<int>((loc.lon + 180.0) / 360.0 * n * TILE_SIZE);
    float radLat = loc.lat * M_PI / 180.0;
    int y = static_cast<int>((1 - std::log(std::tan(radLat) + 1 / std::cos(radLat)) / M_PI) / 2 * n * TILE_SIZE);
    return {x, y};
}

static lv_point_precise_t locToPPx(Location loc, int zoom) {
    int n = 1 << zoom;
    lv_value_precise_t x = ((loc.lon + 180.0) / 360.0 * n * TILE_SIZE);
    float radLat = loc.lat * M_PI / 180.0;
    lv_value_precise_t y = ((1 - std::log(std::tan(radLat) + 1 / std::cos(radLat)) / M_PI) / 2 * n * TILE_SIZE);
    return {x, y};
}

static lv_point_t locToCenterOffsetPx(Location loc, Location centerLoc, int zoom) {
    lv_point_t loc_px = locToPx(loc, zoom);
    lv_point_t center_px = locToPx(centerLoc, zoom);
    int screenX = loc_px.x - center_px.x;
    int screenY = loc_px.y - center_px.y;
    return {screenX, screenY};
}

static lv_point_precise_t locToCenterOffsetPPx(Location loc, Location centerLoc, int zoom) {
    lv_point_precise_t loc_px = locToPPx(loc, zoom);
    lv_point_precise_t center_px = locToPPx(centerLoc, zoom);
    lv_value_precise_t screenX = SCREEN_CENTER_X + loc_px.x - center_px.x;
    lv_value_precise_t screenY = SCREEN_CENTER_Y + loc_px.y - center_px.y;
    return {screenX, screenY};
}

static double haversineDistance(Location loc1, Location loc2) {
    const double R = 6371;
    double dLat = radians(loc2.lat - loc1.lat);
    double dLon = radians(loc2.lon - loc1.lon);
    double a = sin(dLat / 2) * sin(dLat / 2) +
        cos(radians(loc1.lat)) * cos(radians(loc2.lat)) *
        sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
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

static void showSearchDialog(bool visible) {
    if (visible) {
        lv_textarea_set_text(search_field, "");
        lv_obj_remove_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(search_field, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_state(search_field, LV_STATE_FOCUSED);
    } else {
        lv_obj_add_flag(search_field, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

static void showAddrList(bool visible) {
    if (visible) {
        lv_obj_remove_flag(addrs_list, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(addrs_list, LV_OBJ_FLAG_HIDDEN);
    }
}

static void showToast(const char* text) {
    lv_label_set_text(toast, text);
    lv_obj_remove_flag(toast, LV_OBJ_FLAG_HIDDEN);
}

static void hideToast() {
    lv_obj_add_flag(toast, LV_OBJ_FLAG_HIDDEN);
}

static void onClickAddrList(lv_event_t* lv_event);

static void searchAddress(const String& text) {
    foundAddrs.clear();
    String modifiedText = text;
    modifiedText.replace(' ', '%');
    const String query = "SELECT str, num, lon, lat, details FROM addr WHERE alias LIKE '%"
        + modifiedText
        + "%' ORDER BY CAST(num AS INTEGER) ASC LIMIT "
        + ADDR_SEARCH_LIMIT;
    const char* queryCStr = query.c_str();
    sqlite3_exec(addrDb, queryCStr,
                 [](void* data, int argc, char** argv, char** azColName) -> int {
                     const String name = String(argv[0]) + " " + String(argv[1]) + "\n" + String(argv[4]);
                     const float lon = atof(argv[2]);
                     const float lat = atof(argv[3]);
                     foundAddrs.push_back(Address{name, {lon, lat}});
                     return 0;
                 }, (void*)dbData, &zErrMsg);
    lv_obj_clean(addrs_list);
    for (auto addr : foundAddrs) {
        lv_list_add_text(addrs_list, addr.name.c_str());
        lv_obj_t* btn = lv_list_add_btn(addrs_list, NULL, addr.name.c_str());
        lv_obj_add_event_cb(btn, onClickAddrList, LV_EVENT_CLICKED, NULL);
    }
    showAddrList(true);
}

static void update_markers() {
    marker_me.pos = locToCenterOffsetPx(marker_me.loc, centerLoc, zoom);
    lv_obj_set_pos(marker_me.obj, marker_me.pos.x, marker_me.pos.y);

    marker_target.pos = locToCenterOffsetPx(marker_target.loc, centerLoc, zoom);
    lv_obj_set_pos(marker_target.obj, marker_target.pos.x, marker_target.pos.y);
}

static void update_route() {
    if (route.empty()) { return; }
    int idx = 0;
    for (auto& loc : route) {
        route_px[idx++] = locToCenterOffsetPPx(loc, centerLoc, zoom);
    }
    lv_line_set_points(line_route, route_px, idx);
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

    if (lv_obj_has_flag(marker_target.obj, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_state(btn_route, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(btn_route, LV_STATE_DISABLED);
    }
}

static void onDragTile(lv_event_t* e) {
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
    update_route();
    update_buttons();
}

static void onClickAddrList(lv_event_t* e) {
    auto* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
    int index = lv_obj_get_index(btn);
    lv_obj_clear_flag(marker_target.obj, LV_OBJ_FLAG_HIDDEN);
    marker_target.loc = foundAddrs[index].location;
    centerLoc = marker_target.loc;
    if (foundAddrs.size() == 1) showAddrList(false);
    zoom = INIT_ZOOM;
    update_map();
    update_markers();
    update_route();
    update_buttons();
}

static void onClickZoom(lv_event_t* e) {
    void* btn = lv_event_get_target(e);
    int zoom_change = (btn == btn_zoom_in) ? 1 : -1;
    int new_zoom = zoom + zoom_change;
    if (new_zoom == zoom) return;
    lv_cache_drop_all_cb_t();
    zoom = new_zoom;
    update_map();
    update_markers();
    update_route();
    update_buttons();
}

static void onClickTile(lv_event_t* e) {
    lv_indev_t* indev = lv_indev_get_act();
    if (indev == nullptr) return;

    if (!lv_obj_has_flag(search_field, LV_OBJ_FLAG_HIDDEN)) {
        showSearchDialog(false);
        return;
    }

    if (!lv_obj_has_flag(addrs_list, LV_OBJ_FLAG_HIDDEN)) {
        showAddrList(false);
        return;
    }

    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);
    if (abs(vect.x) > DRAG_THRESHOLD || abs(vect.y) > DRAG_THRESHOLD) { return; }
    const auto* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    for (auto& t : tiles) {
        if (t.obj == obj) {
            lv_point_t point;
            lv_indev_get_point(lv_indev_get_act(), &point);
            marker_target.loc = pointToLocation(point, centerLoc, zoom);
            lv_obj_clear_flag(marker_target.obj, LV_OBJ_FLAG_HIDDEN);
            update_markers();
            update_buttons();
            break;
        }
    }
}

static void onClickGps(lv_event_t* e) {
    centerLoc = marker_me.loc;
    zoom = INIT_ZOOM;
    update_map();
    update_markers();
    update_route();
    update_buttons();
}

static void onClickRoute(lv_event_t* e) {
    showToast("Calculating route.\nPlease wait...");
    writeBootState({CURRENT_BM_VER, ModeRoute, centerLoc, zoom, marker_me.loc, marker_target.loc});
    esp_restart();
}

static void onClickSearch(lv_event_t* e) {
    showSearchDialog(true);
}

static void onStartSearch(lv_event_t* e) {
    const char* text = lv_textarea_get_text(search_field);
    if (strlen(text) == 0) return;
    showToast("Searching addresses.\nPlease wait...");
    searchAddress(text);
    showSearchDialog(false);
    hideToast();
}

static lv_obj_t* create_btn(const char* label, const int32_t x, const int32_t y, const lv_event_cb_t onClick, const int32_t w = 40, const int32_t h = 40) {
    lv_obj_t* btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, w, h);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, x, y);
    lv_obj_t* zoom_in_label = lv_label_create(btn);
    lv_label_set_text(zoom_in_label, label);
    lv_obj_center(zoom_in_label);
    lv_obj_add_event_cb(btn, onClick, LV_EVENT_CLICKED, NULL);
    return btn;
}

static void create_route() {
    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 5);
    lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_line_opa(&style_line, 150);
    lv_style_set_line_rounded(&style_line, true);

    line_route = lv_line_create(map_bg);
    lv_obj_add_style(line_route, &style_line, 0);
    lv_obj_set_size(line_route, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(line_route, LV_ALIGN_TOP_LEFT, 0, 0);

    update_route();
    update_buttons();
}

static void create_map() {
    map_bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(map_bg, lv_pct(100), lv_pct(100));
    lv_obj_align(map_bg, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(map_bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(map_bg, 0, 0);

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
#ifdef DEBUG_TILE
            static lv_style_t style;
            lv_style_init(&style);
            lv_style_set_border_width(&style, 1);
            lv_style_set_border_color(&style, lv_color_hex(0xFF0000));
            lv_obj_add_style(t.obj, &style, LV_PART_MAIN);
            lv_obj_set_size(t.obj, TILE_SIZE, ERR_TIMEOUT);
#endif
            lv_obj_align(t.obj, LV_ALIGN_TOP_LEFT, 0, 0);
            lv_obj_add_flag(t.obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(t.obj, onDragTile, LV_EVENT_PRESSING, NULL);
            lv_obj_add_event_cb(t.obj, onClickTile, LV_EVENT_CLICKED, NULL);

            tiles.push_back(t);
        }
    }
    update_map();
}

static void create_buttons() {
    int x = 10, y = 10, step = 50;
    btn_zoom_in = create_btn(LV_SYMBOL_PLUS, x, y, onClickZoom);
    y += step;
    btn_zoom_out = create_btn(LV_SYMBOL_MINUS, x, y, onClickZoom);
    y += step;
    btn_gps = create_btn(LV_SYMBOL_GPS, x, y, onClickGps);
    y += step;
    btn_route = create_btn(LV_SYMBOL_SHUFFLE, x, y, onClickRoute);
    y += step;
    btn_search = create_btn(LV_SYMBOL_HOME, x, y, onClickSearch);

    update_buttons();
}

static void create_markers(Location me, Location target) {
    marker_target = {lv_label_create(map_bg), target, {0, 0}};
    lv_label_set_text(marker_target.obj, LV_SYMBOL_DOWNLOAD);
    lv_obj_set_style_text_color(marker_target.obj, PRIMARY_COLOR, 0);
    lv_obj_align(marker_target.obj, LV_ALIGN_CENTER, 0, 0);
    if (target.lat == 0.0) lv_obj_add_flag(marker_target.obj, LV_OBJ_FLAG_HIDDEN);
    markers.push_back(marker_target);

    Location myLoc = me.lat != 0.0 ? me : centerLoc;
    marker_me = {lv_label_create(map_bg), myLoc, locToCenterOffsetPx(myLoc, centerLoc, zoom)};
    lv_label_set_text(marker_me.obj, LV_SYMBOL_PLAY);
    lv_obj_set_style_transform_angle(marker_me.obj, -900, 0);
    lv_obj_set_style_text_color(marker_me.obj, PRIMARY_COLOR, 0);
    lv_obj_align(marker_me.obj, LV_ALIGN_CENTER, 0, 0);
    markers.push_back(marker_me);

    update_markers();
}

static void create_toast() {
    toast = lv_label_create(lv_screen_active());
    lv_label_set_text(toast, "-");
    lv_obj_set_style_text_align(toast, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_color(toast, TOAST_COLOR, 0);
    lv_obj_set_style_bg_opa(toast, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(toast, 20, 0);
    lv_obj_align(toast, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(toast, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_radius(toast, 6, 0);

    hideToast();
}

static void create_keyboard() {
    static const char* kb_map[] = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
        "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "\n",
        "A", "S", "D", "F", "G", "H", "J", "K", "L", LV_SYMBOL_BACKSPACE, "\n",
        "Z", "X", "C", "V", " ", "B", "N", "M", LV_SYMBOL_OK, NULL
    };

    static const lv_buttonmatrix_ctrl_t kb_ctrl[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 2, 1, 1, 1, 1
    };

    search_field = lv_textarea_create(lv_scr_act());
    lv_obj_align(search_field, LV_ALIGN_TOP_MID, 0, 10);
    lv_textarea_set_placeholder_text(search_field, "Search address");
    lv_obj_set_size(search_field, SCREEN_WIDTH - 20, 40);

    keyboard = lv_keyboard_create(lv_scr_act());
    lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1, kb_map, kb_ctrl);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_USER_1);
    lv_keyboard_set_textarea(keyboard, search_field);
    lv_obj_add_event_cb(search_field, onStartSearch, LV_EVENT_READY, keyboard);

    showSearchDialog(false);
}

static void create_list() {
    addrs_list = lv_list_create(lv_screen_active());
    lv_obj_center(addrs_list);
    lv_obj_add_flag(addrs_list, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(addrs_list, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(addrs_list, &montserrat_14_pl, 0);
    lv_obj_set_size(addrs_list, LV_HOR_RES - 20, LV_VER_RES / 3);
    lv_obj_align(addrs_list, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void Map_init(const BootState& state) {
    LOGI("Init Map");

    sqlite3_initialize();
    sqlite3_open("/sd/addr.db", &addrDb);

    route_px = new lv_point_precise_t[state.route.size()];

    centerLoc = state.center;
    zoom = state.zoom;
    route = state.route;
    distance = state.distance;

    create_map();
    create_markers(state.start, state.end);
    create_buttons();
    create_route();
    create_keyboard();
    create_toast();
    create_list();

    LOG(" ok");
}

