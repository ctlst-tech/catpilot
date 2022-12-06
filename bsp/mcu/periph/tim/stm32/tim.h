#ifndef TIM_H
#define TIM_H

#include <errno.h>

#include "core.h"
#include "gpio.h"
#include "hal.h"
#include "irq.h"

enum tim_state_t {
    TIM_ACTIVE = 0,
    TIM_INACTIVE = 1,
};

typedef struct {
    IRQn_Type id;
    uint32_t counter;
    enum tim_state_t state;
} tim_private_t;

typedef struct {
    TIM_HandleTypeDef init;
    uint32_t scaler_us;
    uint32_t counter_scaled;
    int irq_priority;
    tim_private_t p;
} tim_t;

int tim_init(tim_t *cfg);
void tim_start(tim_t *cfg);
void tim_stop(tim_t *cfg);
uint32_t tim_get_tick(tim_t *cfg);

#endif  // TIM_H
