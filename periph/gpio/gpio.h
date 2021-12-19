#include "stm32_base.h"
#pragma once

typedef struct gpio_cfg_t{
    GPIO_TypeDef *GPIO;
    GPIO_InitTypeDef GPIO_InitStruct;
} gpio_cfg_t;

int GPIO_Init(gpio_cfg_t *cfg);
int GPIO_ClockEnable(gpio_cfg_t *cfg);
