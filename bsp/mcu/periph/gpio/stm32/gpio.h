#ifndef GPIO_H
#define GPIO_H

#include "core.h"
#include "hal.h"

typedef struct gpio_cfg_t {
    GPIO_TypeDef *GPIO;
    GPIO_InitTypeDef GPIO_InitStruct;
} gpio_cfg_t;

int GPIO_Init(gpio_cfg_t *cfg);
int GPIO_ClockEnable(gpio_cfg_t *cfg);

void GPIO_Set(gpio_cfg_t *cfg);
void GPIO_Reset(gpio_cfg_t *cfg);
void GPIO_Toggle(gpio_cfg_t *cfg);
void GPIO_SetState(gpio_cfg_t *cfg, uint8_t state);
int GPIO_Read(gpio_cfg_t *cfg);

#endif  // GPIO_H
