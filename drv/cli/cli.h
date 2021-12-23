#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"

extern usart_cfg_t usart7_cfg;

#define PIN_USART7_RX    {GPIOF, {GPIO_PIN_6, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_AF8_UART7}}
#define PIN_USART7_TX    {GPIOE, {GPIO_PIN_8, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_AF8_UART7}}

int CLI_Init();
