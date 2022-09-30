#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"

int Monitor_Init(tim_cfg_t *tim);
void Monitor_StartTimer(void);
uint32_t Monitor_GetCounter(void);
void Monitor_Update(void);
