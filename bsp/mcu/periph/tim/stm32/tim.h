#ifndef TIM_H
#define TIM_H

#include "core.h"
#include "errno.h"
#include "gpio.h"
#include "hal.h"
#include "irq.h"

enum tim_state_t {
    TIM_ACTIVE,
    TIM_INACTIVE,
};

typedef struct {
    IRQn_Type id;
    uint32_t counter;
    enum tim_state_t state;
} tim_private_t;

typedef struct {
    TIM_HandleTypeDef init;
    uint32_t counter_scaler_us;
    int irq_priority;
    tim_private_t p;
} tim_t;

int tim_init(tim_t *cfg);
void tim_start(tim_t *cfg);
void tim_stop(tim_t *cfg);
uint32_t tim_get_tick(tim_t *cfg);

#endif  // TIM_H
