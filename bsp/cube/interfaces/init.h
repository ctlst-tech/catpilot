#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"
#include "icm20602.h"
#include "px4io.h"
#include "sdcard.h"
#include "cli.h"

extern spi_cfg_t spi1;
extern spi_cfg_t spi2;
extern spi_cfg_t spi4;
extern usart_cfg_t usart6;
extern usart_cfg_t usart7;
extern sdio_cfg_t sdmmc1;
extern adc_cfg_t adc12;

extern gpio_cfg_t gpio_sdcard_pwr;
extern gpio_cfg_t gpio_spi1_cs1;
extern gpio_cfg_t gpio_spi1_cs2;
extern gpio_cfg_t gpio_spi2_cs1;
extern gpio_cfg_t gpio_spi4_cs1;
extern gpio_cfg_t gpio_spi4_cs2;
extern gpio_cfg_t gpio_spi4_cs3;
extern exti_cfg_t exti_spi1_drdy1;

int Board_Init();
int GPIOBoard_Init();
int USART7_Init();
int USART6_Init();
int SPI1_Init();
int SPI2_Init();
int SPI4_Init();
int SDMMC1_Init();
int ADC12_Init();
