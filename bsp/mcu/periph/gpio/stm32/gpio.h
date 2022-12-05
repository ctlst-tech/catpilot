#ifndef GPIO_H
#define GPIO_H

#include <errno.h>

#include "core.h"
#include "hal.h"
#include "os.h"

typedef struct gpio_t {
    GPIO_TypeDef *port;
    GPIO_InitTypeDef init;
} gpio_t;

int gpio_init(gpio_t *cfg);
void gpio_set(gpio_t *cfg);
void gpio_reset(gpio_t *cfg);
void gpio_toggle(gpio_t *cfg);
void gpio_set_state(gpio_t *cfg, uint8_t state);
int gpio_read(gpio_t *cfg);

#endif  // GPIO_H
