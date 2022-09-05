#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"

int IMU_Init(void);
int IO_Init(void);
int MAG_Init(void);

void IMU_Update(void);
void IO_Update(void);
void MAG_Update(void);

int Devices_Init(void);
