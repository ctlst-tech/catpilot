#ifndef EXTI_H
#define EXTI_H

#include "core.h"
#include "gpio.h"
#include "hal.h"
#include "os.h"

typedef struct {
    gpio_t gpio;
    EXTI_ConfigTypeDef cfg;
    EXTI_HandleTypeDef handle;
    IRQn_Type irq;
    int irq_priority;
} exti_t;

int exti_init(exti_t *cfg);
void exti_enable_irq(exti_t *cfg);
void exti_disable_irq(exti_t *cfg);

#endif  // EXTI_H
