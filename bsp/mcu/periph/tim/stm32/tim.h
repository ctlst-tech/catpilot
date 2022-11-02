#ifndef TIM_H
#define TIM_H

#include "core.h"
#include "gpio.h"
#include "hal.h"

enum tim_state_t {
    TIM_ACTIVE,
    TIM_INACTIVE,
};

typedef struct {
    TIM_HandleTypeDef init;
    uint32_t counter;
    uint32_t counter_scaler_us;
    enum tim_state_t state;
    IRQn_Type irq;
    int irq_priority;
} tim_t;

int tim_init(tim_t *cfg);
int tim_clock_enable(tim_t *cfg);
int tim_enable_irq(tim_t *cfg);
int tim_disable_irq(tim_t *cfg);
int tim_handler(tim_t *cfg);

void tim_start(tim_t *cfg);
void tim_stop(tim_t *cfg);
uint32_t tim_get_tick(tim_t *cfg);

#endif  // TIM_H
