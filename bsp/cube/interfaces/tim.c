#include "init.h"
#include "cfg.h"

tim_cfg_t tim2;

int TIM2_Init() {
    int rv;

    tim2.TIM = TIM2;
    tim2.TIM_InitStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim2.TIM_InitStruct.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    tim2.TIM_InitStruct.Init.Prescaler = 200 - 1;
    tim2.TIM_InitStruct.Init.Period = 10;
    tim2.TIM_InitStruct.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    tim2.counter_scaler_us = 10;

    rv = TIM_Init(&tim2);
    TIM_EnableIRQ(&tim2);

    return 0;
}
