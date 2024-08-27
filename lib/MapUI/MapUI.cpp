#include "MapUI.h"
#include "globals.h"
#include "lvgl.h"
#include <BootManager.h>
#include <sqlite3.h>
#include <vector>
#include "montserrat_14_pl.c"
#include "marker.c"
#include "compass.c"
#include "UIhelpers.cpp"

#define DEBUG_MAP 0
#if DEBUG_MAP == 1
#define DEBUG_VALUES
static int DEBUG_VALUE_1 = 0;
static int DEBUG_VALUE_2 = 0;
#endif

LV_FONT_DECLARE(montserrat_14_pl)

#define SYMBOL_ZOOM_IN  "0"
#define SYMBOL_ZOOM_OUT "1"
#define SYMBOL_GPS_OFF  "3"
#define SYMBOL_GPS_ON   "2"
#define SYMBOL_ROUTE    "4"
#define SYMBOL_SEARCH   "5"
#define SYMBOL_MIRROR   "6"
#define SYMBOL_EXIT     "7"

LV_IMG_DECLARE(marker)
LV_IMG_DECLARE(compass)

#define DRAG_THRESHOLD 10
#define FILE_NAME_SIZE 40
#define MARKERS_OPACITY 180
#define MARKER_TARGET_OX (0)
#define MARKER_TARGET_OY (-25)
#define MARKER_ME_OX (0)
#define MARKER_ME_OY (0)
#define ADDR_SEARCH_LIMIT 5
// first tile should be central!
#define TILES_X_SCAN {0,-1,1}
#define TILES_Y_SCAN {0,-1,1}
#define PRIMARY_COLOR lv_color_hex(0x2196F3)
#define TOAST_COLOR lv_color_hex(0xFF5722)

struct UICommand {
    const char* command;
    const char* data;
};

struct Address {
    String name;
    Location location;
    lv_obj_t* btn;
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

static sqlite3* addrDb;
static std::vector<Address> foundAddrs;

static lv_obj_t* btn_zoom_in;
static lv_obj_t* btn_zoom_out;
static lv_obj_t* btn_gps;
static lv_obj_t* btn_route;
static lv_obj_t* btn_search;
static lv_obj_t* btn_mirror;
static lv_obj_t* line_route;
static lv_obj_t* keyboard;
static lv_obj_t* search_field;
static lv_obj_t* toast;
static lv_obj_t* lst_addrs;
static lv_obj_t* lbl_dist;
static Marker marker_me;
static Marker marker_target;
static lv_point_precise_t* route_px;

static std::vector<Tile> tiles;

static std::vector<Location> route = {};
static float distance = -1;

// proto
static void onClickAddrList(lv_event_t* lv_event);

static void showSearchDialog(bool visible) {
    if (visible) {
        lv_textarea_set_text(search_field, "");
        show(keyboard);
        show(search_field);
        lv_obj_add_state(search_field, LV_STATE_FOCUSED);
    } else {
        hide(search_field);
        hide(keyboard);
    }
}

static void showToast(const char* text) {
    lv_label_set_text(toast, text);
    show(toast);
}

static void searchAddress() {
    showSearchDialog(false);

    for (auto& addr : foundAddrs)
        lv_obj_del(addr.btn);

    auto modifiedText = String(lv_textarea_get_text(search_field));
    LOG("Search for", modifiedText);
    modifiedText.replace(' ', '%');
    const String query = "SELECT str, num, lon, lat FROM addr WHERE alias LIKE '%"
        + modifiedText
        + "%' ORDER BY CAST(num AS INTEGER) ASC LIMIT "
        + ADDR_SEARCH_LIMIT;
    const char* queryCStr = query.c_str();
    sqlite3_exec(addrDb, queryCStr,
                 [](void* data, int argc, char** argv, char** azColName) -> int {
                     const String name = String(argv[0]) + " " + String(argv[1]);
                     const float lon = atof(argv[2]);
                     const float lat = atof(argv[3]);
                     foundAddrs.push_back(Address{name, {lon, lat}});
                     return 0;
                 }, (void*)dbData, &zErrMsg);
    hide(toast);
    if (foundAddrs.size() == 0) return;
    for (auto addr : foundAddrs) {
        lv_obj_t* btn = lv_list_add_btn(lst_addrs, NULL, addr.name.c_str());
        lv_obj_add_event_cb(btn, onClickAddrList, LV_EVENT_CLICKED, NULL);
        addr.btn = btn;
    }
    show(lst_addrs);
}

static void updateMarkers(bool onlyMe = false) {
    if (marker_me.obj != nullptr && isVisible(marker_me.obj)) {
        marker_me.pos = locToCenterOffsetPx(marker_me.loc, centerLoc, zoom);
        lv_obj_set_pos(marker_me.obj, marker_me.pos.x + MARKER_ME_OX, marker_me.pos.y + MARKER_ME_OY);
    }
    if (!onlyMe && marker_target.obj != nullptr && isVisible(marker_target.obj)) {
        marker_target.pos = locToCenterOffsetPx(marker_target.loc, centerLoc, zoom);
        lv_obj_set_pos(marker_target.obj, marker_target.pos.x + MARKER_TARGET_OX, marker_target.pos.y + MARKER_TARGET_OY);
    }
}

static void updateRoute() {
    if (route.empty()) { return; }
    int idx = 0;
    for (auto& loc : route) {
        route_px[idx++] = locToCenterOffsetPPx(loc, centerLoc, zoom);
    }
    lv_line_set_points(line_route, route_px, idx);
}

static void updateMap() {
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

static void updateButtons() {
    if (zoom <= ZOOM_MIN) {
        disable(btn_zoom_out);
    } else {
        enable(btn_zoom_out);
    }

    if (zoom >= ZOOM_MAX) {
        disable(btn_zoom_in);
    } else {
        enable(btn_zoom_in);
    }

    if (marker_target.obj != nullptr && isHidden(marker_target.obj)) {
        disable(btn_route);
    } else {
        enable(btn_route);
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

    updateMap();
    updateMarkers();
    updateRoute();
    updateButtons();
}

static void onClickAddrList(lv_event_t* e) {
    auto* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
    int index = lv_obj_get_index(btn);
    show(marker_target.obj);
    const Location newLoc = foundAddrs[index].location;
    LOG("New loc", newLoc.lon, newLoc.lat);
    marker_target.loc = foundAddrs[index].location;
    centerLoc = marker_target.loc;
    if (foundAddrs.size() == 1) hide(lst_addrs);
    updateMap();
    updateMarkers();
    updateRoute();
    updateButtons();
}

static void onClickZoom(lv_event_t* e) {
    void* btn = lv_event_get_target(e);
    int zoom_change = (btn == btn_zoom_in) ? 1 : -1;
    int new_zoom = zoom + zoom_change;
    if (new_zoom == zoom) return;
    lv_cache_drop_all_cb_t();
    zoom = new_zoom;
    updateMap();
    updateMarkers();
    updateRoute();
    updateButtons();
}

static void onClickTile(lv_event_t* e) {
    lv_indev_t* indev = lv_indev_get_act();
    if (indev == nullptr) return;

    if (isVisible(search_field)) {
        showSearchDialog(false);
        return;
    }

    if (isVisible(lst_addrs)) {
        hide(lst_addrs);
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
            LOGF("%.6f, %.6f\n", marker_target.loc.lon, marker_target.loc.lat);
            show(marker_target.obj);
            updateMarkers();
            updateButtons();
            break;
        }
    }
}

static void onClickGps(lv_event_t* e) {
    centerLoc = marker_me.loc;
    zoom = INIT_ZOOM;
    updateMap();
    updateMarkers();
    updateRoute();
    updateButtons();
}

static void onCompassAndGPSTick(lv_timer_t* timer) {
    // todo watch lost connection
    if (isHidden(marker_me.obj)) {
        if (my_location.lon != 0) {
            marker_me.loc = my_location;
            show(marker_me.obj);
            enable(btn_gps);
            onClickGps(nullptr);
        }
    } else {
        marker_me.loc = my_location;
        auto angle_fixed = -(int16_t)(((int)compass_angle - 90) % 360 * 10);
        lv_image_set_rotation(marker_me.obj, angle_fixed);
        updateMarkers(true);
    }
}

static void onClickRoute(lv_event_t* e) {
    hide(lst_addrs);
    showToast("Calculating route.\nPlease wait...");
    lv_timer_create([](lv_timer_t* timer) -> void {
        writeBootState({CURRENT_BM_VER, ModeRoute, centerLoc, zoom, marker_me.loc, marker_target.loc});
        esp_restart();
    }, 100, NULL);
}

static void onClickSearch(lv_event_t* e) {
    hide(lst_addrs);
    showSearchDialog(true);
}

static void onClickMirror(lv_event_t* e) {
}

static void onStartSearch(lv_event_t* e) {
    showSearchDialog(false);
    showToast("Searching address.\nPlease wait...");
    lv_timer_create([](lv_timer_t* timer) -> void {
        lv_timer_del(timer);
        searchAddress();
    }, 100, NULL);
}

static void createRoute() {
    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 5);
    lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_ORANGE));
    lv_style_set_line_opa(&style_line, 150);
    lv_style_set_line_rounded(&style_line, true);

    line_route = lv_line_create(lv_scr_act());
    lv_obj_add_style(line_route, &style_line, 0);
    lv_obj_set_size(line_route, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(line_route, LV_ALIGN_TOP_LEFT, 0, 0);
}

static void createMap() {
    lv_obj_set_size(lv_scr_act(), lv_pct(100), lv_pct(100));
    lv_obj_align(lv_scr_act(), LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

    lv_point_t centerPx = locToPx(centerLoc, zoom);
    int tile_x = centerPx.x / TILE_SIZE;
    int tile_y = centerPx.y / TILE_SIZE;
    int x_offset = centerPx.x % TILE_SIZE;
    int y_offset = centerPx.y % TILE_SIZE;

    for (const int dx : TILES_X_SCAN) {
        for (const int dy : TILES_Y_SCAN) {
            const Tile t = {
                lv_img_create(lv_scr_act()),
                SCREEN_CENTER_X + dx * TILE_SIZE - x_offset,
                SCREEN_CENTER_Y + dy * TILE_SIZE - y_offset,
                tile_x + dx,
                tile_y + dy,
                dx,
                dy,
                0
            };
            lv_obj_align(t.obj, LV_ALIGN_TOP_LEFT, 0, 0);
            lv_obj_add_flag(t.obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(t.obj, onDragTile, LV_EVENT_PRESSING, NULL);
            lv_obj_add_event_cb(t.obj, onClickTile, LV_EVENT_CLICKED, NULL);
            tiles.push_back(t);
        }
    }
}

static void createButtons() {
    int x = 10, y = 10, step = 50;
    btn_zoom_in = createBtn(SYMBOL_ZOOM_IN, x, y, onClickZoom);
    y += step;
    btn_zoom_out = createBtn(SYMBOL_ZOOM_OUT, x, y, onClickZoom);
    y += step;
    btn_gps = createBtn(SYMBOL_GPS_OFF, x, y, onClickGps);
    disable(btn_gps);
    y += step;
    btn_route = createBtn(SYMBOL_ROUTE, x, y, onClickRoute);
    if (distance > 0) {
        String distanceText = String(distance, 1);
        lbl_dist = lv_label_create(btn_route);
        lv_label_set_text(lbl_dist, distanceText.c_str());
        lv_obj_set_style_text_font(lbl_dist, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(lbl_dist, LV_ALIGN_BOTTOM_MID, 0, 9);
    }
    y += step;
    btn_search = createBtn(SYMBOL_SEARCH, x, y, onClickSearch);
    y += step;
    btn_search = createBtn(SYMBOL_MIRROR, x, y, onClickMirror);
}

static void createMarkers(Location me, Location target) {
    const auto img1 = lv_img_create(lv_scr_act());
    lv_image_set_src(img1, &compass);
    lv_obj_center(img1);
    if (my_location.lat == 0.0) hide(img1);
    Location myLoc = me.lat != 0.0 ? me : centerLoc;
    marker_me = {img1, myLoc, locToCenterOffsetPx(myLoc, centerLoc, zoom)};

    const auto img2 = lv_img_create(lv_scr_act());
    lv_image_set_src(img2, &marker);
    lv_obj_center(img2);
    if (target.lat == 0.0) hide(img2);
    marker_target = {img2, target, {0, 0}};
}

static void createToast() {
    toast = lv_label_create(lv_screen_active());
    lv_label_set_text(toast, "-");
    lv_obj_set_style_text_align(toast, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_color(toast, TOAST_COLOR, 0);
    lv_obj_set_style_bg_opa(toast, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(toast, 10, 0);
    lv_obj_align(toast, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_color(toast, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_radius(toast, 6, 0);

    hide(toast);
}

static void createKeyboard() {
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

static void createAddrList() {
    lst_addrs = lv_list_create(lv_screen_active());
    lv_obj_center(lst_addrs);
    hide(lst_addrs);
    lv_obj_set_style_text_font(lst_addrs, &montserrat_14_pl, 0);
    lv_obj_set_width(lst_addrs,LV_HOR_RES - 20);
    lv_obj_set_height(lst_addrs, LV_SIZE_CONTENT);
    lv_obj_align(lst_addrs, LV_ALIGN_BOTTOM_MID, 0, -10);
}

#ifdef DEBUG_VALUES
static void createDebugDashboard() {
    lv_obj_t* slider1 = lv_slider_create(lv_scr_act());
    lv_obj_align(slider1, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_slider_set_value(slider1, 50, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider1, [](lv_event_t* e) -> void {
        lv_obj_t* slider = static_cast<lv_obj_t*>(lv_event_get_target(e));
        int value = lv_slider_get_value(slider);
        DEBUG_VALUE_1 = value - 50;
        LOG("DEBUG_VALUE_1", DEBUG_VALUE_1);
        updateMarkers();
    }, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t* slider2 = lv_slider_create(lv_scr_act());
    lv_obj_align(slider2, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_slider_set_value(slider2, 50, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider2, [](lv_event_t* e) -> void {
        lv_obj_t* slider = static_cast<lv_obj_t*>(lv_event_get_target(e));
        int value = lv_slider_get_value(slider);
        DEBUG_VALUE_2 = value - 50;
        LOG("DEBUG_VALUE_2", DEBUG_VALUE_2);
        updateMarkers();
    }, LV_EVENT_VALUE_CHANGED, NULL);
}
#endif

void Map_init(const BootState& state) {
    LOGI("Init Map");

    sqlite3_initialize();
    sqlite3_open("/sd/addr.db", &addrDb);

    route_px = new lv_point_precise_t[state.route.size()];

    centerLoc = state.center;
    zoom = state.zoom;
    route = state.route;
    distance = state.distance;

    createMap();

#ifdef DEBUG_VALUES
    createDebugDashboard();
#endif

    createRoute();
    createMarkers(state.start, state.end);
    createButtons();
    createKeyboard();
    createToast();
    createAddrList();

    updateMap();
    updateMarkers();
    updateRoute();
    updateButtons();

    lv_timer_create(onCompassAndGPSTick, COMPASS_UPD_PERIOD, NULL);

    LOG(" ok");
}

