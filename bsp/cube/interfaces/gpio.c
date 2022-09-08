#include "init.h"
#include "cfg.h"

gpio_cfg_t gpio_periph_pwr  = GPIO_PERIPH_EN;
gpio_cfg_t gpio_sensors_pwr = GPIO_SENSORS_EN;

gpio_cfg_t gpio_spi1_cs1 = GPIO_SPI1_CS1;
gpio_cfg_t gpio_spi1_cs2 = GPIO_SPI1_CS2;
gpio_cfg_t gpio_spi2_cs1 = GPIO_SPI2_CS1;
gpio_cfg_t gpio_spi4_cs1 = GPIO_SPI4_CS1;
gpio_cfg_t gpio_spi4_cs2 = GPIO_SPI4_CS2;
gpio_cfg_t gpio_spi4_cs3 = GPIO_SPI4_CS3;
exti_cfg_t exti_spi1_drdy1 = EXTI_SPI1_DRDY1;

int GPIOBoard_Init() {
    int rv = 0;

    rv |= GPIO_Init(&gpio_periph_pwr);
    GPIO_Set(&gpio_periph_pwr);
    rv |= GPIO_Init(&gpio_sensors_pwr);
    GPIO_Set(&gpio_sensors_pwr);

    return rv;
}
