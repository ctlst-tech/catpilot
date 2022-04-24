#include "board.h"
#include "board_cfg.h"

gpio_cfg_t gpio_sdcard_pwr = GPIO_SDCARD_PWR;

int GPIOBoard_Init() {
    int rv = 0;

    rv = GPIO_Init(&gpio_sdcard_pwr);
    if(!rv) GPIO_Set(&gpio_sdcard_pwr);

    return rv;
}
