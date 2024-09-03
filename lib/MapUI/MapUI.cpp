#include "MapUI.h"
#include "globals.h"
#include "lvgl.h"
#include <BootManager.h>
#include <HTTPClient.h>
#include <iostream>
#include <sqlite3.h>
#include <sstream>
#include <vector>
#include "Mirror.h"
#include "montserrat_14_pl.c"
#include "marker.c"
#include "compass.c"
#include "UIhelpers.cpp"
#include "Helpers.h"
#include "Touch.h"

#define DEBUG_MAP 0
#if DEBUG_MAP == 1
#define DEBUG_VALUES
static int DEBUG_VALUE_1 = 0;
static int DEBUG_VALUE_2 = 0;
#endif

LV_IMG_DECLARE(marker)
LV_IMG_DECLARE(compass)
LV_FONT_DECLARE(montserrat_14_pl)
// npx lv_font_conv --bpp 4 --size 20 --no-compress --font /Users/buzz/WORK/icons.ttf --range 0x30-0x3d --format lvgl -o /Users/buzz/Projects/navi_c++/lib/fonts/icons.c

#define SYMBOL_ZOOM_IN   "0"
#define SYMBOL_ZOOM_OUT  "1"
#define SYMBOL_DEL       "2"
#define SYMBOL_GPS       "3"
#define SYMBOL_ROUTE     "4"
#define SYMBOL_SEARCH    "5"
#define SYMBOL_MIRROR    "6"
#define SYMBOL_TRIP      "7"
#define SYMBOL_SATELLITE "8"
#define SYMBOL_ALL       "9"
#define SYMBOL_BIKE      ":"
#define SYMBOL_CAR       ";"
#define SYMBOL_WALK      "<"
#define SYMBOL_SETTINGS  "="

#define TIMER_PERIOD 100
#define DRAG_THRESHOLD 10
#define HOLD_TIME_THRESHOLD 20
#define AFTER_DRAG_TIME_THRESHOLD 500
#define FILE_NAME_SIZE 40
#define MARKERS_OPACITY 180
#define MARKER_TARGET_OX (0)
#define MARKER_TARGET_OY (-25)
#define MARKER_ME_OX (0)
#define MARKER_ME_OY (0)
#define ADDR_SEARCH_LIMIT 10
#define TILES_X_SCAN {0,-1,1} // first tile should be central!
#define TILES_Y_SCAN {0,-1,1}
#define COLOR_PRIMARY   lv_palette_main(LV_PALETTE_BLUE)
#define COLOR_SECONDARY lv_palette_main(LV_PALETTE_ORANGE)
#define COLOR_INACTIVE lv_palette_main(LV_PALETTE_GREY)

extern int camWsClientNumber;
extern bool camEnabled;

struct Address {
    String name;
    Location location;
    float distance;
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
static Address foundAddresses[ADDR_SEARCH_LIMIT];
static int foundAddressesCount = 0;

static lv_obj_t* btn_zoom_in;
static lv_obj_t* btn_zoom_out;
static lv_obj_t* btn_route;
static lv_obj_t* btn_del_route;
static lv_obj_t* btn_search;
static lv_obj_t* btn_trip;
static lv_obj_t* ico_gps;
static lv_obj_t* ico_transport;
static lv_obj_t* ico_mirror;
static lv_obj_t* line_route;
static lv_obj_t* keyboard;
static lv_obj_t* ta_search;
static lv_obj_t* lbl_toast;
static lv_obj_t* lbl_distance;
static lv_obj_t* lbl_tooltip;
static lv_obj_t* lst_transport;

static lv_point_precise_t* route_px;
static Marker marker_me;
static Marker marker_target;
static Marker* marker_selected = nullptr;
static Marker markers[ADDR_SEARCH_LIMIT];
static std::vector<Tile> tiles;
static std::vector<Location> route = {};
static float distance = -1;
static Transport transport;

static lv_timer_t* hold_timer = nullptr;
static bool is_hold_valid = false;
static bool tripMode = false;
static unsigned long last_drag_ms = 0;

// proto
static void updateMarkers(bool onlyMe = false);
static void updateMap(bool force = false);
static void updateRoute();
static void updateButtons();
static void onClickMarker(lv_event_t* e);
static void changeMapCenter(Location newLoc, int newZoom);

static void showSearchDialog(const bool visible) {
    if (visible) {
        lv_textarea_set_text(ta_search, "");
        show(keyboard);
        show(ta_search);
        lv_obj_add_state(ta_search, LV_STATE_FOCUSED);
    } else {
        hide(ta_search);
        hide(keyboard);
    }
}

static void showToast(const char* text) {
    lv_label_set_text(lbl_toast, text);
    show(lbl_toast);
}

static void searchAddress() {
    marker_selected = nullptr;
    updateMarkers();
    showSearchDialog(false);
    hide(lbl_tooltip);

    auto modifiedText = String(lv_textarea_get_text(ta_search));
    LOG("Search for", modifiedText);
    modifiedText.replace(' ', '%');
    const String query = "SELECT str, num, lon, lat, "
        "((lat - " + String(marker_me.loc.lat) + ") * (lat - " + String(marker_me.loc.lat) + ") + "
        "(lon - " + String(marker_me.loc.lon) + ") * (lon - " + String(marker_me.loc.lon) + ")) "
        "AS distance FROM addr WHERE alias LIKE '%"
        + modifiedText
        + "%' ORDER BY distance ASC, CAST(num AS INTEGER) ASC LIMIT "
        + ADDR_SEARCH_LIMIT;
    const char* queryCStr = query.c_str();
    foundAddressesCount = 0;
    sqlite3_exec(addrDb, queryCStr,
                 [](void* data, int argc, char** argv, char** azColName) -> int {
                     const String name = String(argv[0]) + " " + String(argv[1]);
                     const float lon = atof(argv[2]);
                     const float lat = atof(argv[3]);
                     foundAddresses[foundAddressesCount++] = {
                         name,
                         {lon, lat},
                         haversineDistance({lon, lat}, marker_me.loc)
                     };
                     return 0;
                 }, (void*)dbData, &zErrMsg);
    hide(lbl_toast);
    if (foundAddressesCount == 0) return;

    int idx = 0;
    bbox_reset();

    for (const auto& addr : foundAddresses) {
        show(markers[idx].obj);
        markers[idx++].loc = addr.location;
        bbox_compare(addr.location);
    }
    bbox_compare(marker_me.loc);

    run_after(100, {
              CenterAndZoom cz = getBBoxCenterAndZoom(bbox_result());
              changeMapCenter(cz.center, cz.zoom);
              })
}

static void changeMapCenter(Location newLoc, int newZoom) {
    centerLoc = newLoc;
    zoom = newZoom;
    updateMap();
    updateMarkers();
    updateRoute();
    updateButtons();
}

static void updateMarkers(bool onlyMe) {
    static lv_point_t pos;
    if (marker_me.obj != nullptr && visible(marker_me.obj)) {
        pos = locToCenterOffsetPx(marker_me.loc, centerLoc, zoom);
        lv_obj_set_pos(marker_me.obj, pos.x + MARKER_ME_OX, pos.y + MARKER_ME_OY);
    }
    if (onlyMe) return;

    if (marker_target.obj != nullptr && visible(marker_target.obj)) {
        pos = locToCenterOffsetPx(marker_target.loc, centerLoc, zoom);
        lv_obj_set_pos(marker_target.obj, pos.x + MARKER_TARGET_OX, pos.y + MARKER_TARGET_OY);
    }
    for (int idx = 0; idx < foundAddressesCount; idx++) {
        pos = locToCenterOffsetPx(markers[idx].loc, centerLoc, zoom);
        lv_obj_set_pos(markers[idx].obj, pos.x + MARKER_TARGET_OX, pos.y + MARKER_TARGET_OY);
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

    if (marker_selected == nullptr && hidden(marker_target.obj)) {
        disable(btn_route);
    } else {
        enable(btn_route);
    }
}

static void onGpsFound() {
    lv_obj_set_style_text_color(ico_gps, COLOR_PRIMARY, 0);
}

static void onGpsLost() {
    lv_obj_set_style_text_color(ico_gps,COLOR_INACTIVE, 0);
}

static void onTimerTick(lv_timer_t* _) {
    if (my_gps_location.lon != 0) {
        marker_me.loc = my_gps_location;
        if (tripMode && getDistanceMeters(my_gps_location, centerLoc) > 2) {
            changeMapCenter(my_gps_location, zoom);
        }
        if (!prevGpsStateReady) onGpsFound();
        prevGpsStateReady = true;
    } else {
        if (prevGpsStateReady) onGpsLost();
        prevGpsStateReady = false;
    }

    const auto angle_fixed = -static_cast<int16_t>((static_cast<int>(compass_angle) + COMPASS_ANGLE_CORRECTION) % 360 * 10);
    lv_image_set_rotation(marker_me.obj, angle_fixed);
    updateMarkers(true);

    if (mtZoom.ready) {
        int newZoom = zoom + mtZoom.direction;
        if (newZoom <= ZOOM_MAX && zoom >= ZOOM_MIN) {
            const Location newLoc = pointToLocation(mtZoom.center, centerLoc, zoom);
            changeMapCenter(newLoc, newZoom);
        }
        mtZoom.ready = false;
    }
}

static void onDragTile(lv_event_t* e) {
    const lv_indev_t* indev = lv_indev_get_act();
    if (indev == NULL) return;
    lv_point_t delta;
    lv_indev_get_vect(indev, &delta);

    if (abs(delta.x) <= DRAG_THRESHOLD && abs(delta.y) <= DRAG_THRESHOLD) return;

    const lv_point_t centerPx = locToPx(centerLoc, zoom);
    const lv_point_t newCenterPx = {centerPx.x - delta.x, centerPx.y - delta.y};
    changeMapCenter(pxToLoc(newCenterPx, zoom), zoom);
    last_drag_ms = millis();
}

static void onClickZoom(lv_event_t* e) {
    void* btn = lv_event_get_target(e);
    int zoom_change = (btn == btn_zoom_in) ? 1 : -1;
    int new_zoom = zoom + zoom_change;
    if (new_zoom == zoom) return;
    changeMapCenter(centerLoc, new_zoom);
}

static void onClickMarker(lv_event_t* e) {
    hide(lbl_tooltip);
    const auto* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    int idx = 0;
    for (auto& m : markers) {
        if (m.obj == obj) {
            if (marker_selected != nullptr) {
                lv_obj_set_style_img_recolor_opa(marker_selected->obj, LV_OPA_TRANSP, 0);
            }
            if (marker_selected == &m) { // second click
                changeMapCenter(m.loc, ZOOM_DEFAULT);
            }
            // first click
            marker_selected = &m;

            const char* name = foundAddresses[idx].name.c_str();
            size_t name_length = strlen(name);
            char buffer[name_length + 20];

            float distance = foundAddresses[idx].distance;
            if (distance < 1.0) {
                const int meters = static_cast<int>(distance * 1000);
                snprintf(buffer, sizeof(buffer), "%s\n%d m", name, meters);
            } else {
                snprintf(buffer, sizeof(buffer), "%s\n%.1f km", name, distance);
            }
            lv_label_set_text(lbl_tooltip, buffer);


            lv_obj_set_style_img_recolor_opa(marker_selected->obj, LV_OPA_COVER, 0);
            lv_obj_set_style_img_recolor(marker_selected->obj, COLOR_SECONDARY, 0);
            lv_obj_align_to(lbl_tooltip, obj, LV_ALIGN_OUT_TOP_MID, 0, -5);

            lv_coord_t max_width = SCREEN_WIDTH - 20;

            lv_obj_set_width(lbl_tooltip, LV_SIZE_CONTENT);
            if (lv_obj_get_width(lbl_tooltip) > max_width) lv_obj_set_width(lbl_tooltip, max_width);
            lv_obj_align_to(lbl_tooltip, obj, LV_ALIGN_OUT_TOP_MID, 0, -5);
            lv_area_t tooltip_area;
            lv_obj_get_coords(lbl_tooltip, &tooltip_area);

            if (tooltip_area.y1 < 10) {
                lv_obj_align_to(lbl_tooltip, obj, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
            }

            if (tooltip_area.x1 < 0) {
                lv_obj_set_x(lbl_tooltip, 10);
            } else if (tooltip_area.x2 > SCREEN_WIDTH) {
                lv_obj_set_x(lbl_tooltip, SCREEN_WIDTH - lv_obj_get_width(lbl_tooltip) - 10);
            }
            hide(marker_target.obj);
            show(lbl_tooltip);
            break;
        }
        idx++;
    }
}

static void onClickTile(lv_event_t* e) {
    const lv_event_code_t code = lv_event_get_code(e);
    const lv_indev_t* indev = lv_indev_get_act();
    if (indev == nullptr) return;

    if (visible(ta_search)) {
        showSearchDialog(false);
        return;
    }

    if (visible(lbl_tooltip)) {
        hide(lbl_tooltip);
        return;
    }

    if (distance > 0) return;

    // check click to add target marker
    if (millis() - last_drag_ms < AFTER_DRAG_TIME_THRESHOLD) return;

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

        // add target marker  todo: make fn selectTargetMarker()
        const auto* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
        for (const auto& t : tiles) {
            if (t.obj == obj) {
                lv_point_t point;
                lv_indev_get_point(lv_indev_get_act(), &point);
                marker_target.loc = pointToLocation(point, centerLoc, zoom);
                LOGF("%.6f, %.6f\n", marker_target.loc.lon, marker_target.loc.lat);
                show(marker_target.obj);

                if (marker_selected != nullptr) {
                    lv_obj_set_style_img_recolor_opa(marker_selected->obj, LV_OPA_TRANSP, 0);
                    marker_selected = nullptr;
                }

                updateMarkers();
                updateButtons();
                break;
            }
        }
    }
}

static void onClickGps(lv_event_t* e) {
    changeMapCenter(marker_me.loc, ZOOM_DEFAULT);
}

static void onClickRoute(lv_event_t* e) {
    showToast("Calculating route.\nPlease wait...");
    run_after(100, {
              Location start = marker_me.loc;
              Location end = marker_selected ? marker_selected->loc : marker_target.loc;
              writeBootState({CURRENT_BM_VER, ModeRoute, transport, centerLoc, zoom, start, end});
              esp_restart();
              })
}

static void onClickDelRoute(lv_event_t* e) {
    writeBootState({CURRENT_BM_VER, ModeMap, TransportAll, centerLoc, zoom, marker_me.loc, marker_target.loc});
    route.clear();
    distance = -1;
    updateButtons();
    lv_obj_del(line_route);
    lv_obj_del(lbl_distance);
    hide(btn_del_route);
    show(btn_route);
}

static void onClickSearch(lv_event_t* e) {
    for (int idx = 0; idx < ADDR_SEARCH_LIMIT; idx++) {
        hide(markers[idx].obj);
    }
    updateMarkers();
    showSearchDialog(true);
}

static void onClickMirror(lv_event_t* e) {
    if (camEnabled) {
        Mirror_stop();
        lv_obj_set_style_text_color(ico_mirror, COLOR_INACTIVE, 0);
        run_after(500, updateMap(true))
    } else {
        Mirror_start();
        lv_obj_set_style_text_color(ico_mirror, COLOR_PRIMARY, 0);
    }
}

static void onClickTrip(lv_event_t* e) {
    tripMode = !tripMode;
    if (tripMode) changeMapCenter(marker_me.loc, ZOOM_TRIP);
    lv_obj_set_style_text_color(btn_trip, tripMode ? COLOR_PRIMARY : COLOR_INACTIVE, 0);
}

static void onClickTransportList(lv_event_t* e) {
    auto btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
    lv_obj_t* list = lv_obj_get_parent(btn);

    if (lv_obj_check_type(list, &lv_list_class)) {
        uint32_t index = lv_obj_get_index(btn);
        transport = getTransportByIdx(index);
        lv_label_set_text(ico_transport, getSymbolByTransport(transport));
        hide(lst_transport);
    }
}

static void onClickTransportIco(lv_event_t* e) {
    show(lst_transport);
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
    lv_style_set_line_color(&style_line, COLOR_PRIMARY);
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
    int x = SCREEN_WIDTH - 45, y = SCREEN_HEIGHT - 30, step = -45;
    btn_zoom_in = createBtn(SYMBOL_ZOOM_IN, x, y, onClickZoom);
    x += step;
    btn_zoom_out = createBtn(SYMBOL_ZOOM_OUT, x, y, onClickZoom);
    x += step;
    btn_search = createBtn(SYMBOL_SEARCH, x, y, onClickSearch);
    x += step;
    btn_route = createBtn(SYMBOL_ROUTE, x, y, onClickRoute);
    if (distance > 0)
        hide(btn_route);
}

static void createMarkers(Location start, Location target) {
    const auto img2 = lv_img_create(lv_scr_act());
    lv_image_set_src(img2, &marker);
    lv_obj_center(img2);
    lv_obj_set_style_img_recolor_opa(img2, LV_OPA_COVER, 0);
    lv_obj_set_style_img_recolor(img2, lv_palette_main(LV_PALETTE_RED), 0);
    if (target.lat == 0.0)
        hide(img2);
    marker_target = {img2, target};

    for (int idx = 0; idx < ADDR_SEARCH_LIMIT; idx++) {
        const auto img = lv_img_create(lv_scr_act());
        lv_image_set_src(img, &marker);
        lv_obj_center(img);
        lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(img, onClickMarker, LV_EVENT_PRESSED, NULL);
        markers[idx] = {img, {0, 0}};
        hide(img);
    }
    const auto img1 = lv_img_create(lv_scr_act());
    lv_image_set_src(img1, &compass);
    lv_obj_center(img1);
    Location myLoc = start.lat != 0.0 ? start : centerLoc;
    marker_me = {img1, myLoc};
}

static void createToast() {
    lbl_toast = lv_label_create(lv_scr_act());
    lv_label_set_text(lbl_toast, "-");
    lv_obj_set_style_text_align(lbl_toast, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_color(lbl_toast, COLOR_SECONDARY, 0);
    lv_obj_set_style_bg_opa(lbl_toast, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(lbl_toast, 10, 0);
    lv_obj_align(lbl_toast, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_text_color(lbl_toast, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_radius(lbl_toast, 6, 0);
    hide(lbl_toast);
}

static void createTooltip() {
    lbl_tooltip = lv_label_create(lv_scr_act());
    lv_obj_set_style_bg_color(lbl_tooltip, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(lbl_tooltip, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(lbl_tooltip, lv_color_black(), 0);
    lv_obj_set_style_text_color(lbl_tooltip, lv_color_black(), 0);
    lv_obj_set_style_border_width(lbl_tooltip, 1, 0);
    lv_obj_center(lbl_tooltip);
    lv_obj_set_style_text_font(lbl_tooltip, &montserrat_14_pl, 0);
    lv_obj_set_style_pad_all(lbl_tooltip, 10, 0);
    lv_obj_set_style_radius(lbl_tooltip, 6, 0);
    lv_label_set_long_mode(lbl_tooltip, LV_LABEL_LONG_WRAP);
    hide(lbl_tooltip);
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

    ta_search = lv_textarea_create(lv_scr_act());
    lv_obj_align(ta_search, LV_ALIGN_TOP_MID, 0, 10);
    lv_textarea_set_placeholder_text(ta_search, "Search address");
    lv_obj_set_size(ta_search, SCREEN_WIDTH - 20, 40);

    keyboard = lv_keyboard_create(lv_scr_act());
    lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1, kb_map, kb_ctrl);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_USER_1);
    lv_keyboard_set_textarea(keyboard, ta_search);
    lv_obj_add_event_cb(ta_search, onStartSearch, LV_EVENT_READY, keyboard);

    showSearchDialog(false);
}

static void createStatusBar() {
    int step = -50, x = 0, y = 0;

    const auto bg = lv_label_create(lv_scr_act());
    lv_obj_set_style_bg_color(bg, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(bg, LV_OPA_50, 0);
    lv_obj_align(bg, LV_ALIGN_TOP_MID, 0, 0);
    lv_label_set_text(bg, "");
    lv_obj_set_size(bg, SCREEN_WIDTH, 30);

    if (distance > 0) {
        char buffer[20];
        if (distance < 1.0) {
            const int meters = static_cast<int>(distance * 1000);
            snprintf(buffer, sizeof(buffer), "%d m", meters);
        } else {
            snprintf(buffer, sizeof(buffer), "%.1f km", distance);
        }
        lbl_distance = lv_label_create(lv_scr_act());
        lv_label_set_text(lbl_distance, buffer);
        lv_obj_set_style_text_color(lbl_distance, lv_color_black(), 0);
        lv_obj_align(lbl_distance, LV_ALIGN_TOP_LEFT, 34, 8);

        btn_del_route = createStatusIcon(SYMBOL_DEL, 0, 0, onClickDelRoute);
        lv_obj_align(btn_del_route, LV_ALIGN_TOP_LEFT, -10, y);
        lv_obj_set_style_text_color(btn_del_route, lv_palette_main(LV_PALETTE_RED), 0);
    }

    ico_transport = createStatusIcon(getSymbolByTransport(transport), x, y, onClickTransportIco);
    x += step;
    ico_gps = createStatusIcon(SYMBOL_SATELLITE, x, y, onClickGps);
    lv_obj_set_style_text_color(ico_gps, COLOR_INACTIVE, 0);
    x += step;
    ico_mirror = createStatusIcon(SYMBOL_MIRROR, x, y, onClickMirror);
    lv_obj_set_style_text_color(ico_mirror, COLOR_INACTIVE, 0);
    x += step;
    if (distance > 0) {
        btn_trip = createStatusIcon(SYMBOL_TRIP, x, y, onClickTrip);
        lv_obj_set_style_text_color(btn_trip, COLOR_INACTIVE, 0);
    }
}

void createTransportList() {
    lst_transport = lv_list_create(lv_screen_active());
    lv_obj_align(lst_transport, LV_ALIGN_TOP_RIGHT, -5, 20);
    lv_obj_set_height(lst_transport, LV_SIZE_CONTENT);
    lv_obj_t* btn;
    btn = lv_list_add_button(lst_transport, NULL, "All");
    lv_obj_add_event_cb(btn, onClickTransportList, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_button(lst_transport, NULL, "Walk");
    lv_obj_add_event_cb(btn, onClickTransportList, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_button(lst_transport, NULL, "Bike");
    lv_obj_add_event_cb(btn, onClickTransportList, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_button(lst_transport, NULL, "Car");
    lv_obj_add_event_cb(btn, onClickTransportList, LV_EVENT_CLICKED, NULL);
    hide(lst_transport);
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
    transport = state.transport;

    createMap();
    createRoute();
    createMarkers(state.start, state.end);
    createButtons();
    createStatusBar();
    createKeyboard();
    createToast();
    createTooltip();
    createTransportList();

    changeMapCenter(state.center, state.zoom);

    // run_after(1000, {
    //           lv_textarea_set_text(ta_search,"lidl");
    //           searchAddress();
    //           })

#ifdef DEBUG_VALUES
    createDebugDashboard();
#endif

    lv_timer_create(onTimerTick, TIMER_PERIOD, NULL);

    LOG(" ok");
}

