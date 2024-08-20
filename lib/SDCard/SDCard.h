
#ifndef WATCH_SD
#define WATCH_SD

#define MOUNT_POINT "/sd"

#define PIN_NUM_MISO  GPIO_NUM_15
#define PIN_NUM_MOSI  GPIO_NUM_17
#define PIN_NUM_CLK   GPIO_NUM_16
#define PIN_NUM_CS    GPIO_NUM_7

class SDCard {
public:
    static void init();

private:
};

#endif
