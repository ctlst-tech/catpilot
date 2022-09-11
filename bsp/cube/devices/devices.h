#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"

int IMU_INT_Init(void);
int IMU_EXT_Init(void);
int IO_Init(void);
int MAG_Init(void);

void IMU_INT_Update(void);
void IMU_EXT_Update(void);
void IO_Update(void);
void MAG_Update(void);

int Devices_Init(void);
