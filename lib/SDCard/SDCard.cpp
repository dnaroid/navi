#include "SDCard.h"
#include <esp_err.h>
#include <esp_vfs_fat.h>
#include <globals.h>
#include <sdmmc_cmd.h>

void SDCard_init() {
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 10,
        .allocation_unit_size = 64 * 1024
    };
    sdmmc_card_t* card;
    const char mount_point[] = MOUNT_POINT;
    LOG("Initializing SD card\n");

    sdmmc_host_t host = SDSPI_HOST_CUSTOM();
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(spi_host_device_t::SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        printf("Failed to initialize bus.\n");
        return;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_CUSTOM();

    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = spi_host_device_t::SPI2_HOST;

    printf("Mounting filesystem\n");

    do {
        ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

        if (ret != ESP_OK) {
            if (ret == ESP_FAIL) {
                LOG("Failed to mount filesystem. ");
            } else {
                LOG("Failed to initialize the card (%s). ", esp_err_to_name(ret));
            }
        }
        delay(100);
    } while (ret != ESP_OK);

    printf("Filesystem mounted\n");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
}
