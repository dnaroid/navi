
#ifndef WATCH_SD
#define WATCH_SD

#define MOUNT_POINT "/sd"

#define SDSPI_DEVICE_CONFIG() {\
.host_id   = SPI2_HOST, \
.gpio_cs   = GPIO_NUM_18, \
.gpio_cd   = SDSPI_SLOT_NO_CD, \
.gpio_wp   = SDSPI_SLOT_NO_WP, \
.gpio_int  = GPIO_NUM_NC, \
}

#define PIN_NUM_MISO  GPIO_NUM_15
#define PIN_NUM_MOSI  GPIO_NUM_17
#define PIN_NUM_CLK   GPIO_NUM_16
#define PIN_NUM_CS    GPIO_NUM_7

void SDCard_init();

#endif
