#ifndef BOARD_H
#define BOARD_H

#include "periph.h"

int board_clock_init(void);
int board_periph_init(void);
int board_services_start(void);

extern gpio_t gpio_brick1_valid;
extern gpio_t gpio_brick2_valid;
extern gpio_t gpio_usb_valid;
extern gpio_t gpio_sensors_en;
extern gpio_t gpio_periph_en;
extern gpio_t gpio_periph_oc;
extern gpio_t gpio_hipwr_oc;
extern gpio_t gpio_adc_inp14;
extern gpio_t gpio_adc_inp15;
extern gpio_t gpio_adc_inp18;
extern gpio_t gpio_adc_inp13;
extern gpio_t gpio_adc_inp4;
extern gpio_t gpio_adc_inp8;
extern gpio_t gpio_spi1_sck;
extern gpio_t gpio_spi1_miso;
extern gpio_t gpio_spi1_mosi;
extern gpio_t gpio_spi1_cs1;
extern exti_t exti_spi1_drdy1;
extern gpio_t gpio_spi1_cs2;
extern gpio_t gpio_spi2_sck;
extern gpio_t gpio_spi2_miso;
extern gpio_t gpio_spi2_mosi;
extern gpio_t gpio_spi2_cs1;
extern gpio_t gpio_spi4_sck;
extern gpio_t gpio_spi4_miso;
extern gpio_t gpio_spi4_mosi;
extern gpio_t gpio_spi4_cs1;
extern gpio_t gpio_spi4_cs2;
extern gpio_t gpio_spi4_cs3;
extern gpio_t gpio_spi4_cs4;
extern gpio_t gpio_sdmmc1_ck;
extern gpio_t gpio_sdmmc1_cmd;
extern gpio_t gpio_sdmmc1_d0;
extern gpio_t gpio_sdmmc1_d1;
extern gpio_t gpio_sdmmc1_d2;
extern gpio_t gpio_sdmmc1_d3;
extern gpio_t gpio_sdmmc1_cd;
extern gpio_t gpio_usart7_tx;
extern gpio_t gpio_usart7_rx;
extern gpio_t gpio_usart6_tx;
extern gpio_t gpio_usart6_rx;
extern gpio_t gpio_usart4_tx;
extern gpio_t gpio_usart4_rx;
extern gpio_t gpio_usart8_tx;
extern gpio_t gpio_usart8_rx;
extern gpio_t gpio_usart2_tx;
extern gpio_t gpio_usart2_rx;
extern gpio_t gpio_usart3_tx;
extern gpio_t gpio_usart3_rx;
extern gpio_t gpio_fmu_pwm[6];

extern usart_t usart6;

#endif  // BOARD_H
