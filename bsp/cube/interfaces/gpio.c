#include "init.h"
#include "cfg.h"

gpio_cfg_t gpio_periph_pwr  = GPIO_PERIPH_EN;
gpio_cfg_t gpio_sensors_pwr = GPIO_SENSORS_EN;

int GPIOBoard_Init() {
    int rv = 0;

    rv |= GPIO_Init(&gpio_periph_pwr);
    GPIO_Set(&gpio_periph_pwr);
    rv |= GPIO_Init(&gpio_sensors_pwr);
    GPIO_Set(&gpio_sensors_pwr);

    return rv;
}
