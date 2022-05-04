#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"

extern spi_cfg_t spi1;
extern i2c_cfg_t i2c3;
extern usart_cfg_t usart7;
extern usart_cfg_t usart8;
extern sdio_cfg_t sdmmc1;

extern gpio_cfg_t gpio_sdcard_pwr;
extern gpio_cfg_t gpio_spi1_cs1;
extern gpio_cfg_t gpio_spi1_cs2;
extern gpio_cfg_t gpio_spi1_cs3;
extern gpio_cfg_t gpio_spi1_cs4;
extern exti_cfg_t exti_spi1_drdy1;
extern exti_cfg_t exti_spi1_drdy2;
extern exti_cfg_t exti_spi1_drdy3;
extern exti_cfg_t exti_spi1_drdy4;

int Board_Init();
int GPIOBoard_Init();
int USART7_Init();
int USART8_Init();
int SPI1_Init();
void SPI1_ChipSelect(int num);
void SPI1_ChipDeselect();
int I2C3_Init();
int SDMMC1_Init();
