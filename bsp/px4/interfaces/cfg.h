#pragma once
#include "stm32_base.h"

// Power Control
#define GPIO_SDCARD_PWR   {GPIOG, {GPIO_PIN_7,  GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_HIGH, GPIO_AF0_MCO}}
#define GPIO_PERIPH_PWR   {GPIOG, {GPIO_PIN_4,  GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_HIGH, GPIO_AF0_MCO}}
#define GPIO_HI_PWR       {GPIOF, {GPIO_PIN_12, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_HIGH, GPIO_AF0_MCO}}

// Debug USART7 bus
#define GPIO_USART7_RX    {GPIOF, {GPIO_PIN_6, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, GPIO_AF8_UART7}}
#define GPIO_USART7_TX    {GPIOE, {GPIO_PIN_8, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, GPIO_AF8_UART7}}

// Sensors SPI1 bus
#define GPIO_SPI1_MISO    {GPIOA, {GPIO_PIN_6,  GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF5_SPI1}}
#define GPIO_SPI1_MOSI    {GPIOD, {GPIO_PIN_7,  GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF5_SPI1}}
#define GPIO_SPI1_SCK     {GPIOG, {GPIO_PIN_11, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF5_SPI1}}
// ICM20689
#define GPIO_SPI1_CS1     {GPIOF, {GPIO_PIN_2,  GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_AF5_SPI1}}
#define GPIO_SPI1_DRDY1   {GPIOB, {GPIO_PIN_4,  GPIO_MODE_IT_RISING, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0}}
#define EXTI_SPI1_DRDY1   {GPIO_SPI1_DRDY1, {EXTI_LINE_4, EXTI_MODE_INTERRUPT, EXTI_TRIGGER_RISING, EXTI_GPIOB}, {EXTI_LINE_4, NULL}, 6, 0}
// ICM20602
#define GPIO_SPI1_CS2     {GPIOF, {GPIO_PIN_3,  GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_AF5_SPI1}}
#define GPIO_SPI1_DRDY2   {GPIOC, {GPIO_PIN_5,  GPIO_MODE_IT_RISING, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0}}
#define EXTI_SPI1_DRDY2   {GPIO_SPI1_DRDY2, {EXTI_LINE_5, EXTI_MODE_INTERRUPT, EXTI_TRIGGER_RISING, EXTI_GPIOC}, {EXTI_LINE_5, NULL}, 6, 0}
// BMI055_GYR
#define GPIO_SPI1_CS3     {GPIOF, {GPIO_PIN_4,  GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_AF5_SPI1}}
#define GPIO_SPI1_DRDY3   {GPIOB, {GPIO_PIN_14,  GPIO_MODE_IT_RISING, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0}}
#define EXTI_SPI1_DRDY3   {GPIO_SPI1_DRDY3, {EXTI_LINE_14, EXTI_MODE_INTERRUPT, EXTI_TRIGGER_RISING, EXTI_GPIOB}, {EXTI_LINE_14, NULL}, 6, 0}
// BMI055_ACC
#define GPIO_SPI1_CS4     {GPIOG, {GPIO_PIN_10, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_AF5_SPI1}}
#define GPIO_SPI1_DRDY4   {GPIOB, {GPIO_PIN_15,  GPIO_MODE_IT_RISING, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0}}
#define EXTI_SPI1_DRDY4   {GPIO_SPI1_DRDY4, {EXTI_LINE_15, EXTI_MODE_INTERRUPT, EXTI_TRIGGER_RISING, EXTI_GPIOB}, {EXTI_LINE_15, NULL}, 6, 0}

// IST8310 I2C3 bus
#define GPIO_I2C3_SDA     {GPIOH, {GPIO_PIN_8,  GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF4_I2C3}}
#define GPIO_I2C3_SCL     {GPIOH, {GPIO_PIN_7,  GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF4_I2C3}}

// SD card SDMMC1 bus
#define GPIO_SDMMC1_CK    {GPIOC, {GPIO_PIN_12, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_HIGH, GPIO_AF12_SDMMC1}}
#define GPIO_SDMMC1_CMD   {GPIOD, {GPIO_PIN_2,  GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_HIGH, GPIO_AF12_SDMMC1}}
#define GPIO_SDMMC1_D0    {GPIOC, {GPIO_PIN_8,  GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_HIGH, GPIO_AF12_SDMMC1}}
#define GPIO_SDMMC1_D1    {GPIOC, {GPIO_PIN_9,  GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_HIGH, GPIO_AF12_SDMMC1}}
#define GPIO_SDMMC1_D2    {GPIOC, {GPIO_PIN_10, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_HIGH, GPIO_AF12_SDMMC1}}
#define GPIO_SDMMC1_D3    {GPIOC, {GPIO_PIN_11, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_HIGH, GPIO_AF12_SDMMC1}}
#define GPIO_SDMMC1_CD    {GPIOG, {GPIO_PIN_0,  GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_HIGH, GPIO_AF0_MCO}}

// PX4IO USART8 bus
#define GPIO_USART8_RX    {GPIOE, {GPIO_PIN_0, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, GPIO_AF8_UART8}}
#define GPIO_USART8_TX    {GPIOE, {GPIO_PIN_1, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, GPIO_AF8_UART8}}