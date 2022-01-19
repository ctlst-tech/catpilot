#pragma once
#include "stm32_base.h"
#include "gpio.h"

typedef struct {
    gpio_cfg_t gpio;
    EXTI_ConfigTypeDef EXTI_ConfigStruct;
    EXTI_HandleTypeDef EXTI_Handle;
    int priority;
    IRQn_Type IRQ;
} exti_cfg_t;

int EXTI_Init(exti_cfg_t *cfg);
int EXTI_EnableIRQ(exti_cfg_t *cfg);
