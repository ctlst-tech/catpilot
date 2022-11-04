#ifndef EXTI_H
#define EXTI_H

#include "core.h"
#include "dma.h"
#include "gpio.h"
#include "hal.h"
#include "irq.h"
#include "os.h"

typedef struct {
    IRQn_Type id;
} exti_private_t;

typedef struct {
    gpio_t gpio;
    EXTI_ConfigTypeDef cfg;
    EXTI_HandleTypeDef handle;
    int irq_priority;
    exti_private_t p;
} exti_t;

int exti_init(exti_t *cfg);

#endif  // EXTI_H
