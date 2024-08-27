#define USER_SETUP_INFO "User_Setup"

#define USE_HSPI_PORT

#define DISABLE_ALL_LIBRARY_WARNINGS

#define TFT_WIDTH  320
#define TFT_HEIGHT 480

#define ST7796_DRIVER

// #define RPI_DISPLAY_TYPE

#define TFT_MISO 13
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10
#define TFT_DC   14  // Data Command control pin
#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST

#define TOUCH_CS -1

#define SPI_FREQUENCY  80000000

