#ifndef EXTI_H
#define EXTI_H

#include "core.h"
#include "gpio.h"
#include "hal.h"

typedef struct {
    gpio_cfg_t gpio;
    EXTI_ConfigTypeDef EXTI_ConfigStruct;
    EXTI_HandleTypeDef EXTI_Handle;
    int priority;
    IRQn_Type IRQ;
} exti_cfg_t;

int EXTI_Init(exti_cfg_t *cfg);
void EXTI_EnableIRQ(exti_cfg_t *cfg);
void EXTI_DisableIRQ(exti_cfg_t *cfg);

#endif  // EXTI_H
