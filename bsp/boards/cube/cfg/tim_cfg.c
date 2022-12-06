#include "board.h"
#include "core.h"
#include "hal.h"

// MONITOR
// 10kHz
tim_t tim2 = {
    .init.Instance = TIM2,
    .init.Init.CounterMode = TIM_COUNTERMODE_UP,
    .init.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1,
    .init.Init.Prescaler = 199,
    .init.Init.Period = 10,
    .init.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE,
    .scaler_us = 10,
    .irq_priority = 15
};
