#ifndef BOARD_H
#define BOARD_H

#include "cubeio.h"
#include "icm20602.h"
#include "icm20649.h"
#include "icm20948.h"
#include "ist8310.h"
#include "ms5611.h"
#include "periph.h"
#include "sd.h"

#define BOARD_MAX_USART 6
#define BOARD_MAX_CLI_DEVICES 10

int board_start(int (*callback)(void), size_t stacksize, char *cli_port,
                char *cli_baudrate);

extern uint32_t *board_monitor_counter;

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
extern gpio_t gpio_i2c1_scl;
extern gpio_t gpio_i2c1_sda;
extern gpio_t gpio_i2c2_scl;
extern gpio_t gpio_i2c2_sda;
extern gpio_t gpio_can1_tx;
extern gpio_t gpio_can1_rx;
extern gpio_t gpio_can2_tx;
extern gpio_t gpio_can2_rx;
extern gpio_t gpio_otg_fs_dp;
extern gpio_t gpio_otg_fs_dm;
extern gpio_t gpio_otg_fs_vbus;
extern gpio_t gpio_otg_fs_sof;
extern gpio_t gpio_otg_fs_id;

extern sdio_t sdio;
extern spi_t spi1;
extern spi_t spi4;

extern tim_t tim2;

extern usart_t *usart[6];
extern usart_t usart2;
extern usart_t usart3;
extern usart_t usart4;
extern usart_t usart6;
extern usart_t usart7;
extern usart_t usart8;

extern i2c_t i2c1;
extern i2c_t i2c2;

extern icm20649_t *icm20649;
extern icm20602_t *icm20602;
extern icm20948_t *icm20948;
extern cubeio_t *cubeio;
extern ms5611_t *ms5611_1;
extern ms5611_t *ms5611_2;
extern ist8310_t *ist8310;

extern adc_t adc1;

extern can_t can1;
extern can_t can2;

extern usb_t usb0;

extern periph_base_t *cli_dev[BOARD_MAX_CLI_DEVICES];

#endif  // BOARD_H
