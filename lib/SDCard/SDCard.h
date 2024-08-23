
#ifndef WATCH_SD
#define WATCH_SD

#define MOUNT_POINT "/sd"

#define SDSPI_DEVICE_CONFIG_CUSTOM() {\
.host_id   = SPI2_HOST, \
.gpio_cs   = GPIO_NUM_7, \
.gpio_cd   = SDSPI_SLOT_NO_CD, \
.gpio_wp   = SDSPI_SLOT_NO_WP, \
.gpio_int  = GPIO_NUM_NC, \
}

#define SDSPI_HOST_CUSTOM() {\
.flags = SDMMC_HOST_FLAG_SPI | SDMMC_HOST_FLAG_DEINIT_ARG, \
.slot = SDSPI_DEFAULT_HOST, \
.max_freq_khz = 80000, \
.io_voltage = 3.3f, \
.init = &sdspi_host_init, \
.set_bus_width = NULL, \
.get_bus_width = NULL, \
.set_bus_ddr_mode = NULL, \
.set_card_clk = &sdspi_host_set_card_clk, \
.set_cclk_always_on = NULL, \
.do_transaction = &sdspi_host_do_transaction, \
.deinit_p = &sdspi_host_remove_device, \
.io_int_enable = &sdspi_host_io_int_enable, \
.io_int_wait = &sdspi_host_io_int_wait, \
.command_timeout_ms = 0, \
}

#define PIN_NUM_MISO  GPIO_NUM_15
#define PIN_NUM_MOSI  GPIO_NUM_17
#define PIN_NUM_CLK   GPIO_NUM_16
#define PIN_NUM_CS    GPIO_NUM_7

void SDCard_init();

#endif
