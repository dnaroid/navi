#define USER_SETUP_INFO "User_Setup"


#define DISABLE_ALL_LIBRARY_WARNINGS
// #define LOAD_GLCD
// #define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
// #define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
// #define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
// #define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.

#ifdef MINI_TFT

#define ST7789_DRIVER

#define TFT_RGB_ORDER TFT_RGB

#define TFT_WIDTH  240
#define TFT_HEIGHT 280

#define USE_HSPI_PORT

#define TFT_CS            9
#define TFT_DC            8
#define TFT_SCLK         10
#define TFT_MOSI         11
#define TFT_MISO         12
#define TFT_RST          14

#define TOUCH_CS -1

#define SPI_FREQUENCY  80000000
#else

#define USE_HSPI_PORT

#define ST7796_DRIVER

#define TFT_WIDTH  320
#define TFT_HEIGHT 480

#define TFT_MISO 13
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10
#define TFT_DC   14  // Data Command control pin
#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST

#define TOUCH_CS -1
#define SPI_FREQUENCY  80000000
#endif


