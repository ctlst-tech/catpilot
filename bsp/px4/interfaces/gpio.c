#include "init.h"
#include "cfg.h"

gpio_cfg_t gpio_sdcard_pwr = GPIO_SDCARD_PWR;
gpio_cfg_t gpio_spi1_cs1 = GPIO_SPI1_CS1;
gpio_cfg_t gpio_spi1_cs2 = GPIO_SPI1_CS2;
gpio_cfg_t gpio_spi1_cs3 = GPIO_SPI1_CS3;
gpio_cfg_t gpio_spi1_cs4 = GPIO_SPI1_CS4;
exti_cfg_t exti_spi1_drdy1 = EXTI_SPI1_DRDY1;
exti_cfg_t exti_spi1_drdy2 = EXTI_SPI1_DRDY2;
exti_cfg_t exti_spi1_drdy3 = EXTI_SPI1_DRDY3;
exti_cfg_t exti_spi1_drdy4 = EXTI_SPI1_DRDY4;

int GPIOBoard_Init() {
    int rv = 0;

    rv |= GPIO_Init(&gpio_sdcard_pwr);
    GPIO_Set(&gpio_sdcard_pwr);

    rv |= GPIO_Init(&gpio_spi1_cs1);
    rv |= GPIO_Init(&gpio_spi1_cs2);
    rv |= GPIO_Init(&gpio_spi1_cs3);
    rv |= GPIO_Init(&gpio_spi1_cs4);
    GPIO_Set(&gpio_spi1_cs1);
    GPIO_Set(&gpio_spi1_cs2);
    GPIO_Set(&gpio_spi1_cs3);
    GPIO_Set(&gpio_spi1_cs4);

    rv |= EXTI_Init(&exti_spi1_drdy1);
    rv |= EXTI_Init(&exti_spi1_drdy2);
    rv |= EXTI_Init(&exti_spi1_drdy3);
    rv |= EXTI_Init(&exti_spi1_drdy4);
    EXTI_DisableIRQ(&exti_spi1_drdy1);
    EXTI_DisableIRQ(&exti_spi1_drdy2);
    EXTI_DisableIRQ(&exti_spi1_drdy3);
    EXTI_DisableIRQ(&exti_spi1_drdy4);

    if(rv) printf("GPIO initialization failed");

    return rv;
}
