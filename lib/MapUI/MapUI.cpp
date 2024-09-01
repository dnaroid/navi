#include "MapUI.h"
#include "globals.h"
#include "lvgl.h"
#include <BootManager.h>
#include <HTTPClient.h>
#include <sqlite3.h>
#include <vector>
#include "Mirror.h"
#include "montserrat_14_pl.c"
#include "marker.c"
#include "compass.c"
#include "UIhelpers.cpp"
#include "Helpers.h"

#define DEBUG_MAP 0
#if DEBUG_MAP == 1
#define DEBUG_VALUES
static int DEBUG_VALUE_1 = 0;
static int DEBUG_VALUE_2 = 0;
#endif

LV_IMG_DECLARE(marker)
LV_IMG_DECLARE(compass)
LV_FONT_DECLARE(montserrat_14_pl)

#define SYMBOL_ZOOM_IN   "0"
#define SYMBOL_ZOOM_OUT  "1"
#define SYMBOL_DEL       "2"
#define SYMBOL_GPS       "3"
#define SYMBOL_ROUTE     "4"
#define SYMBOL_SEARCH    "5"
#define SYMBOL_MIRROR    "6"
#define SYMBOL_EXIT      "7"
#define SYMBOL_SATELLITE "8"
#define SYMBOL_ALL       "9"
#define SYMBOL_BIKE      ":"
#define SYMBOL_CAR       ";"
#define SYMBOL_WALK      "<"
#define SYMBOL_SETTINGS  "="

#define TIMER_PERIOD 100
#define DRAG_THRESHOLD 10
#define HOLD_TIME_THRESHOLD 20
#define FILE_NAME_SIZE 40
#define MARKERS_OPACITY 180
#define MARKER_TARGET_OX (0)
#define MARKER_TARGET_OY (-25)
#define MARKER_ME_OX (0)
#define MARKER_ME_OY (0)
#define ADDR_SEARCH_LIMIT 5
#define TILES_X_SCAN {0,-1,1} // first tile should be central!
#define TILES_Y_SCAN {0,-1,1}
#define COLOR_PRIMARY   lv_palette_main(LV_PALETTE_BLUE)
#define COLOR_SECONDARY lv_palette_main(LV_PALETTE_ORANGE)
#define COLOR_INACTIVE lv_palette_main(LV_PALETTE_GREY)

extern int camWsClientNumber;
extern bool camEnabled;

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
};

static Location centerLoc;
static bool prevGpsStateReady = false;
static int zoom = 0;

static char* zErrMsg = nullptr;
static auto dbData = "Callback function called";

static sqlite3* addrDb;
static std::vector<Address> foundAddrs;

static lv_obj_t* btn_zoom_in;
static lv_obj_t* btn_zoom_out;
static lv_obj_t* btn_gps;
static lv_obj_t* btn_route;
static lv_obj_t* btn_del_route;
static lv_obj_t* btn_search;
static lv_obj_t* btn_mirror;

static lv_obj_t* ico_gps;
static lv_obj_t* ico_transport;
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
static std::vector<Marker> markers;
static std::vector<Location> route = {};
static float distance = -1;

static lv_timer_t* hold_timer = nullptr;
static bool is_hold_valid = false;

// proto
static void onClickAddrList(lv_event_t*);
static void updateMarkers(bool onlyMe = false);
static void updateMap(bool force = false);
static void updateRoute();
static void updateButtons();


static void showSearchDialog(const bool visible) {
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

    float minLon = std::numeric_limits<float>::max();
    float maxLon = std::numeric_limits<float>::lowest();
    float minLat = std::numeric_limits<float>::max();
    float maxLat = std::numeric_limits<float>::lowest();

    for (auto addr : foundAddrs) {
        const auto img = lv_img_create(lv_scr_act());
        lv_image_set_src(img, &marker);
        lv_obj_center(img);
        markers.push_back({img, addr.location});
        if (addr.location.lon < minLon) minLon = addr.location.lon;
        if (addr.location.lon > maxLon) maxLon = addr.location.lon;
        if (addr.location.lat < minLat) minLat = addr.location.lat;
        if (addr.location.lat > maxLat) maxLat = addr.location.lat;

        lv_obj_t* btn = lv_list_add_btn(lst_addrs, NULL, addr.name.c_str());
        lv_obj_add_event_cb(btn, onClickAddrList, LV_EVENT_CLICKED, NULL);
        addr.btn = btn;
    }
    if (marker_me.loc.lon < minLon) minLon = marker_me.loc.lon;
    if (marker_me.loc.lon > maxLon) maxLon = marker_me.loc.lon;
    if (marker_me.loc.lat < minLat) minLat = marker_me.loc.lat;
    if (marker_me.loc.lat > maxLat) maxLat = marker_me.loc.lat;

    const CenterAndZoom cz = getBBoxCenterAndZoom({minLon, maxLon, minLat, maxLat});
    centerLoc = cz.center;
    zoom = cz.zoom;
    updateMap();
    updateMarkers();
    updateRoute();
    updateButtons();
    // show(lst_addrs);
}

static void updateMarkers(bool onlyMe) {
    static lv_point_t pos;
    if (marker_me.obj != nullptr && visible(marker_me.obj)) {
        pos = locToCenterOffsetPx(marker_me.loc, centerLoc, zoom);
        lv_obj_set_pos(marker_me.obj, pos.x + MARKER_ME_OX, pos.y + MARKER_ME_OY);
    }
    if (!onlyMe && marker_target.obj != nullptr && visible(marker_target.obj)) {
        pos = locToCenterOffsetPx(marker_target.loc, centerLoc, zoom);
        lv_obj_set_pos(marker_target.obj, pos.x + MARKER_TARGET_OX, pos.y + MARKER_TARGET_OY);
    }
    for (auto& m : markers) {
        pos = locToCenterOffsetPx(m.loc, centerLoc, zoom);
        lv_obj_set_pos(m.obj, pos.x + MARKER_TARGET_OX, pos.y + MARKER_TARGET_OY);
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

static void updateMap(bool force) {
    const lv_point_t centerPx = locToPx(centerLoc, zoom);
    const int tile_x = centerPx.x / TILE_SIZE;
    const int tile_y = centerPx.y / TILE_SIZE;
    const int x_offset = centerPx.x % TILE_SIZE;
    const int y_offset = centerPx.y % TILE_SIZE;
    const bool changedTile = tiles[0].tile_x != tile_x || tiles[0].tile_y != tile_y || tiles[0].zoom != zoom || force;

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

    if (marker_target.obj != nullptr && hidden(marker_target.obj)) {
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

    const lv_point_t centerPx = locToPx(centerLoc, zoom);
    const lv_point_t newCenterPx = {centerPx.x - delta.x, centerPx.y - delta.y};
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
    if (foundAddrs.size() == 1)
        hide(lst_addrs);
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
    const lv_event_code_t code = lv_event_get_code(e);
    const lv_indev_t* indev = lv_indev_get_act();
    if (indev == nullptr) return;

    if (visible(search_field)) {
        showSearchDialog(false);
        return;
    }

    if (visible(lst_addrs)) {
        hide(lst_addrs);
        return;
    }

    if (code == LV_EVENT_PRESSED) {
        is_hold_valid = false;
        run_after(HOLD_TIME_THRESHOLD, is_hold_valid = true;)
    } else if (code == LV_EVENT_RELEASED) {
        if (hold_timer) {
            lv_timer_del(hold_timer);
            hold_timer = nullptr;
        }

        if (!is_hold_valid) return;

        lv_point_t delta;
        lv_indev_get_vect(indev, &delta);
        if (abs(delta.x) > DRAG_THRESHOLD || abs(delta.y) > DRAG_THRESHOLD) return; // drag detected

        const auto* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
        for (const auto& t : tiles) {
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
}

static void onClickGps(lv_event_t* e) {
    centerLoc = marker_me.loc;
    zoom = INIT_ZOOM;
    updateMap();
    updateMarkers();
    updateRoute();
    updateButtons();
}

static void onGpsFound() {
    lv_obj_set_style_text_color(ico_gps,COLOR_PRIMARY, 0);
}

static void onGpsLost() {
    lv_obj_set_style_text_color(ico_gps,COLOR_INACTIVE, 0);
}

static void onTimerTick(lv_timer_t* _) {
    if (my_location.lon != 0) {
        marker_me.loc = my_location;
        if (!prevGpsStateReady) onGpsFound();
        prevGpsStateReady = true;
    } else {
        if (prevGpsStateReady) onGpsLost();
        prevGpsStateReady = false;
    }

    const auto angle_fixed = -static_cast<int16_t>((static_cast<int>(compass_angle) + COMPASS_ANGLE_CORRECTION) % 360 * 10);
    lv_image_set_rotation(marker_me.obj, angle_fixed);
    updateMarkers(true);
}

static void onClickRoute(lv_event_t* e) {
    hide(lst_addrs);
    showToast("Calculating route.\nPlease wait...");
    run_after(100, {
              writeBootState({CURRENT_BM_VER, ModeRoute, centerLoc, zoom, marker_me.loc, marker_target.loc});
              esp_restart();
              })
}

static void onClickDelRoute(lv_event_t* e) {
    writeBootState({CURRENT_BM_VER, ModeMap, centerLoc, zoom, marker_me.loc, marker_target.loc});
    route.clear();
    distance = 0;
    lv_obj_del(line_route);
    hide(btn_del_route);
    show(btn_route);
}

static void onClickSearch(lv_event_t* e) {
    hide(lst_addrs);
    showSearchDialog(true);
}

static void onClickMirror(lv_event_t* e) {
    if (camEnabled) {
        Mirror_stop();
        lv_obj_remove_state(btn_mirror, LV_STATE_PRESSED);
        run_after(500, updateMap(true))
    } else {
        Mirror_start();
        lv_obj_add_state(btn_mirror, LV_STATE_PRESSED);
    }
}

static void onStartSearch(lv_event_t* e) {
    showSearchDialog(false);
    showToast("Searching address.\nPlease wait...");
    run_after(100, searchAddress())
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
            lv_obj_add_event_cb(t.obj, onClickTile, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(t.obj, onClickTile, LV_EVENT_RELEASED, NULL);
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
    btn_gps = createBtn(SYMBOL_GPS, x, y, onClickGps);
    y += step;
    btn_route = createBtn(SYMBOL_ROUTE, x, y, onClickRoute);
    if (distance > 0) {
        btn_del_route = createBtn(SYMBOL_DEL, x, y, onClickDelRoute);
        const auto distanceText = String(distance, 1);
        lbl_dist = lv_label_create(btn_del_route);
        lv_label_set_text(lbl_dist, distanceText.c_str());
        lv_obj_set_style_text_font(lbl_dist, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(lbl_dist, LV_ALIGN_BOTTOM_MID, 0, 7);
        lv_obj_align(lv_obj_get_child(btn_del_route, 0), LV_ALIGN_TOP_MID, 0, -7);
        hide(btn_route);
    }
    y += step;
    btn_search = createBtn(SYMBOL_SEARCH, x, y, onClickSearch);
    y += step;
    btn_mirror = createBtn(SYMBOL_MIRROR, x, y, onClickMirror);
}

static void createMarkers(Location start, Location target) {
    const auto img1 = lv_img_create(lv_scr_act());
    lv_image_set_src(img1, &compass);
    lv_obj_center(img1);
    Location myLoc = start.lat != 0.0 ? start : centerLoc;
    marker_me = {img1, myLoc};

    const auto img2 = lv_img_create(lv_scr_act());
    lv_image_set_src(img2, &marker);
    lv_obj_center(img2);
    if (target.lat == 0.0)
        hide(img2);
    marker_target = {img2, target};
}

static void createToast() {
    toast = lv_label_create(lv_scr_act());
    lv_label_set_text(toast, "-");
    lv_obj_set_style_text_align(toast, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_color(toast, COLOR_SECONDARY, 0);
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
    lst_addrs = lv_list_create(lv_scr_act());
    lv_obj_center(lst_addrs);
    hide(lst_addrs);
    lv_obj_set_style_text_font(lst_addrs, &montserrat_14_pl, 0);
    lv_obj_set_width(lst_addrs,LV_HOR_RES - 20);
    lv_obj_set_height(lst_addrs, LV_SIZE_CONTENT);
    lv_obj_align(lst_addrs, LV_ALIGN_BOTTOM_MID, 0, -10);
}

static void createStatusBar() {
    int step = -30, x = -2, y = 2;

    ico_transport = lv_label_create(lv_scr_act());
    lv_label_set_text(ico_transport, SYMBOL_ALL);
    lv_obj_set_style_text_font(ico_transport, &icons, 0);
    lv_obj_set_style_text_color(ico_transport,COLOR_PRIMARY, 0);
    lv_obj_align(ico_transport, LV_ALIGN_TOP_RIGHT, x, y);
    lv_obj_add_flag(ico_transport, LV_OBJ_FLAG_CLICKABLE);
    x += step;

    ico_gps = lv_label_create(lv_scr_act());
    lv_label_set_text(ico_gps, SYMBOL_SATELLITE);
    lv_obj_set_style_text_font(ico_gps, &icons, 0);
    lv_obj_set_style_text_color(ico_gps,COLOR_INACTIVE, 0);
    lv_obj_align(ico_gps, LV_ALIGN_TOP_RIGHT, x, y);
    disable(ico_gps);
    x += step;
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
    createStatusBar();

    updateMap();
    updateMarkers();
    updateRoute();
    updateButtons();

    lv_timer_create(onTimerTick, TIMER_PERIOD, NULL);

    LOG(" ok");
}

