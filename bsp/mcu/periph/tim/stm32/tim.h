#ifndef TIM_H
#define TIM_H

#include "core.h"
#include "gpio.h"
#include "hal.h"

enum tim_state_t {
    TIM_ACTIVE,
    TIM_INACTIVE,
};

struct tim_inst_t {
    IRQn_Type IRQ;
    enum tim_state_t state;
};

typedef struct {
    TIM_TypeDef *TIM;
    TIM_HandleTypeDef TIM_InitStruct;
    struct tim_inst_t inst;
    int priority;
    uint32_t counter;
    uint32_t counter_scaler_us;
} tim_cfg_t;

int TIM_Init(tim_cfg_t *cfg);
int TIM_ClockEnable(tim_cfg_t *cfg);
int TIM_EnableIRQ(tim_cfg_t *cfg);
int TIM_DisableIRQ(tim_cfg_t *cfg);
int TIM_Handler(tim_cfg_t *cfg);

void TIM_Start(tim_cfg_t *cfg);
void TIM_Stop(tim_cfg_t *cfg);
uint32_t TIM_GetTick(tim_cfg_t *cfg);

#endif  // TIM_H
