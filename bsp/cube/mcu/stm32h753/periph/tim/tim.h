#pragma once
#include "stm32_base.h"
#include "gpio.h"

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
} tim_cfg_t;

int TIM_Init(tim_cfg_t *cfg);
int TIM_ClockEnable(tim_cfg_t *cfg);
int TIM_EnableIRQ(tim_cfg_t *cfg);
int TIM_DisableIRQ(tim_cfg_t *cfg);

void TIM_Start(tim_cfg_t *cfg);
void TIM_Stop(tim_cfg_t *cfg);
void TIM_GetTick(tim_cfg_t *cfg);
