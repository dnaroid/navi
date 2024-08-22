
#ifndef RD_H
#define RD_H
#include <lv_api_map_v8.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>
#include <hal/uart_types.h>
#include <widgets/chart/lv_chart.h>

#define GPS_MAX_SATELLITES_IN_USE (12)
#define GPS_MAX_SATELLITES_IN_VIEW (16)
#define NMEA_MAX_STATEMENT_ITEM_LENGTH (16)

class WatchGps {
public:
    typedef enum {
        GPS_FIX_INVALID, /*!< Not fixed */
        GPS_FIX_GPS, /*!< GPS */
        GPS_FIX_DGPS, /*!< Differential GPS */
    } gps_fix_t;

    typedef enum {
        GPS_MODE_INVALID = 1, /*!< Not fixed */
        GPS_MODE_2D, /*!< 2D GPS */
        GPS_MODE_3D /*!< 3D GPS */
    } gps_fix_mode_t;

    typedef struct {
        uint8_t num; /*!< Satellite number */
        uint8_t elevation; /*!< Satellite elevation */
        uint16_t azimuth; /*!< Satellite azimuth */
        uint8_t snr; /*!< Satellite signal noise ratio */
    } gps_satellite_t;

    typedef struct {
        uint8_t hour; /*!< Hour */
        uint8_t minute; /*!< Minute */
        uint8_t second; /*!< Second */
        uint16_t thousand; /*!< Thousand */
    } gps_time_t;

    typedef struct {
        uint8_t day; /*!< Day (start from 1) */
        uint8_t month; /*!< Month (start from 1) */
        uint16_t year; /*!< Year (start from 2000) */
    } gps_date_t;

    typedef enum {
        STATEMENT_UNKNOWN = 0, /*!< Unknown statement */
        STATEMENT_GGA, /*!< GGA */
        STATEMENT_GSA, /*!< GSA */
        STATEMENT_RMC, /*!< RMC */
        STATEMENT_GSV, /*!< GSV */
        STATEMENT_GLL, /*!< GLL */
        STATEMENT_VTG, /*!< VTG */
        STATEMENT_GPTXT, /*!< GPTXT */
        STATEMENT_GNZDA /*!< GNZDA */
    } nmea_statement_t;

    typedef struct {
        float latitude; /*!< Latitude (degrees) */
        float longitude; /*!< Longitude (degrees) */
        float altitude; /*!< Altitude (meters) */
        gps_fix_t fix; /*!< Fix status */
        uint8_t sats_in_use; /*!< Number of satellites in use */
        gps_time_t tim; /*!< time in UTC */
        gps_fix_mode_t fix_mode; /*!< Fix mode */
        uint8_t sats_id_in_use[GPS_MAX_SATELLITES_IN_USE]; /*!< ID list of satellite in use */
        float dop_h; /*!< Horizontal dilution of precision */
        float dop_p; /*!< Position dilution of precision  */
        float dop_v; /*!< Vertical dilution of precision  */
        uint8_t sats_in_view; /*!< Number of satellites in view */
        gps_satellite_t sats_desc_in_view[GPS_MAX_SATELLITES_IN_VIEW]; /*!< Information of satellites in view */
        gps_date_t date; /*!< Fix date */
        bool valid; /*!< GPS validity */
        float speed; /*!< Ground speed, unit: m/s */
        float cog; /*!< Course over ground */
        float variation; /*!< Magnetic variation */
    } gps_t;

    typedef struct esp_gps_t {
        uint8_t item_pos; /*!< Current position in item */
        uint8_t item_num; /*!< Current item number */
        uint8_t asterisk; /*!< Asterisk detected flag */
        uint8_t crc; /*!< Calculated CRC value */
        uint8_t parsed_statement; /*!< OR'd of statements that have been parsed */
        uint8_t sat_num; /*!< Satellite number */
        uint8_t sat_count; /*!< Satellite count */
        uint8_t cur_statement; /*!< Current statement ID */
        uint32_t all_statements; /*!< All statements mask */
        char item_str[NMEA_MAX_STATEMENT_ITEM_LENGTH]; /*!< Current item */
        gps_t parent; /*!< Parent class */
        uart_port_t uart_port; /*!< Uart port number */
        uint8_t* buffer; /*!< Runtime buffer */
        TaskHandle_t tsk_hdl; /*!< NMEA Parser task handle */
        QueueHandle_t event_queue; /*!< UART event queue handle */
    } esp_gps_t;

    typedef struct {
        struct {
            uart_port_t uart_port; /*!< UART port number */
            uint32_t rx_pin; /*!< UART Rx Pin number */
            uint32_t baud_rate; /*!< UART baud rate */
            uart_word_length_t data_bits; /*!< UART data bits length */
            uart_parity_t parity; /*!< UART parity */
            uart_stop_bits_t stop_bits; /*!< UART stop bits length */
            uint32_t event_queue_size; /*!< UART event queue size */
        } uart; /*!< UART specific configuration */
    } nmea_parser_config_t;

    typedef void* nmea_parser_handle_t;

    typedef enum {
        GPS_UPDATE, /*!< GPS information has been updated */
        GPS_UNKNOWN /*!< Unknown statements detected */
    } nmea_event_id_t;

    static bool gps_enabled;
    static bool gps_fixed;

    static bool gps_tracking_stop;
    static uint32_t gps_point_saved;
    static gps_t gps_saved;
    static gps_fix_mode_t gps_status;
    static esp_gps_t global_gps;
    // static float gps_latitude;
    // static float gps_longitude;
    // static float gps_altitude;
    // static float gps_speed;
    // static char gps_datetime[21];
    // static char gps_date[11];
    // static char gps_time[9];

    static void tracking_task(void* pvParameter);

    static nmea_parser_handle_t nmea_parser_init(const nmea_parser_config_t* config);

    static double gps_lat;
    static double gps_lon;
    static uint32_t lon_to_tile_x(double lon);
    static uint32_t lat_lon_to_tile_y(double lat, double lon);
    static uint32_t lon_to_x(double lon);
    static uint32_t lat_lon_to_y(double lat, double lon);

    static void set_state_gps(bool state);

private:
    static esp_gps_t* esp_gps;
    // static void gps_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void nmea_parser_task_entry(void* arg);
    static esp_err_t gps_decode(uint8_t* buff_uart, size_t len);
    static esp_err_t parse_item(esp_gps_t* esp_gps);
    static void parse_gptxt(esp_gps_t* esp_gps);
    static void parse_vtg(esp_gps_t* esp_gps);
    static void parse_gll(esp_gps_t* esp_gps);
    static void parse_gnzda(esp_gps_t* esp_gps);
    static void parse_rmc(esp_gps_t* esp_gps);
    static void parse_gsv(esp_gps_t* esp_gps);
    static void parse_gsa(esp_gps_t* esp_gps);
    static void parse_gga(esp_gps_t* esp_gps);
    static void parse_utc_time(esp_gps_t* esp_gps);
    static float parse_lat_long(esp_gps_t* esp_gps);
    static inline uint8_t convert_two_digit2number(const char* digit_char);
};

class WatchTft {
public:
    enum class screen_index_t {
        main = 1,
        power = 2,
        run = 3,
        settings = 4,
        gps = 5,
    };

    static bool screen_en;
    static TaskHandle_t current_task_hanlde;

    static const lv_img_dsc_t download;

    static void init();

    static void turn_screen_on();
    static void turn_screen_off();
    static void turn_off_screen();
    static void set_battery_text(uint8_t percent);
    static void toggle_button_menu_view();

    static void main_screen_from_sleep();
    static void add_chart_power_value(lv_coord_t batt, lv_coord_t vbus);

    static void set_gps_info(bool fixed, uint8_t in_use, float dop, double lat, double lon);

    static void refresh_tile(int32_t x, int32_t y);
    static void add_gps_pos(int32_t x, int32_t y);

    static void set_wifi_state(bool enable, bool connected);
    static void set_gps_state(bool enable, bool fixed);

    static void set_charge_state(bool state);

private:
    enum class LCD_CMD : uint32_t {
        TURN_OFF_SCREEN,
        SLEEP_SCREEN,
        RESET_ESP,
        RETURN_HOME,
        POWER_SCREEN,
        RUN_SCREEN,
        SETTINGS_SCREEN,
        GPS_SCREEN,
    };

    enum class LCD_BTN_EVENT : uint32_t {
        TURN_OFF_SCREEN,
        SLEEP_SCREEN,
        MAIN_PAGE,
        NEXT_PAGE,
        PREVIOUS_PAGE,
        RESET_ESP,
        TOP_BAR_TOGGLE,
        RETURN_HOME,
        POWER_SCREEN,
        RUN_SCREEN,
        SETTINGS_SCREEN,
        GPS_SCREEN,
        GPS_TRACKING,
        CENTER_GPS,
        CLEAN_FILE,
    };

    static lv_obj_t* current_screen;
    static lv_obj_t* top_menu;
    static lv_obj_t* button_menu;

    // images
    static const lv_img_dsc_t lune;
    static const lv_img_dsc_t ampoule;
    static const lv_img_dsc_t main_bg;
    static const lv_img_dsc_t gradient_count;
    static const lv_img_dsc_t rotate;
    static const lv_img_dsc_t arrow;
    static const lv_img_dsc_t circle;
    static const lv_img_dsc_t run;

    // top menu
    static lv_obj_t* label_battery;
    static lv_obj_t* slider_top_bl;
    static uint8_t s_battery_percent;

    // power screen
    static lv_obj_t* chart_power;
    static lv_obj_t* label_chart_usb;
    static lv_obj_t* label_chart_bat;
    static lv_chart_series_t* power_chart_ser_bat;
    static lv_chart_series_t* power_chart_ser_usb;

    // run screen
    static lv_obj_t* label_step_count;
    static lv_obj_t* arc_counter;

    // settings screen
    static lv_obj_t* label_desc_tmo_off;
    static lv_obj_t* label_desc_step_goal;

    // gps screen
    static lv_obj_t* label_title_gps;
    static lv_obj_t* label_gps_info;
    static lv_obj_t* gps_recording_indicator;
    static lv_obj_t* btn_start_tracking;

    // physical button menu
    static lv_obj_t* lcd_turn_off_screen;
    static lv_obj_t* lcd_reset_screen;
    static lv_obj_t* lcd_sleep_screen;

    static uint8_t bl_value;
    static int bckl_handle;
    static lv_style_t img_recolor_white_style;

    static SemaphoreHandle_t xGuiSemaphore;
    static QueueHandle_t xQueueLcdCmd;

    static lv_obj_t* img_wifi;
    static lv_obj_t* line_wifi;
    static lv_obj_t* img_gps;
    static lv_obj_t* line_gps;

    static lv_point_t wifi_line_points[2];
    static lv_point_t gps_line_points[2];

    static screen_index_t current_screen_index;

    static void lv_tick_task(void* arg);
    static void gui_task(void* pvParameter);
    static void queue_cmd_task(void* pvParameter);
    static void init_home_screen(void);
    static void slider_event_cb(lv_event_t* e);
    static void set_bl(uint8_t value);
    static void toggle_top_bar_view();
    static void event_handler_main(lv_event_t* e);
    static void sleep_screen();
    static void reset_screen();
    static void power_screen();
    static void load_screen(screen_index_t screen_index);
    static void load_main_screen();
    static void load_top_bar(const char* title);
    static void load_button_menu();

    static void run_screen();
    static void count_step_task(void* pvParameter);
    static void set_step_counter_value(uint32_t steps);

    static void settings_screen();
    static void slider_tmo_off_event_cb(lv_event_t* e);
    static void wakeup_double_tap_event_cb(lv_event_t* e);
    static void wakeup_tilt_event_cb(lv_event_t* e);
    static void slider_step_goal_event_cb(lv_event_t* e);

    static void gps_screen();
    static void center_gps();
    static lv_point_t drag_view(int32_t x, int32_t y, bool sem_taken = false);
    static void drag_event_handler(lv_event_t* e);
    static void add_xy_to_gps_point(int32_t x, int32_t y, bool sem_taken = false);

    static void top_menu_event_handler(lv_event_t* e);
};
#endif //RD_H
