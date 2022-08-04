#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"

#include "const.h"

typedef struct {
    float mag_scale;
} ist8310_param_t;

typedef struct {
    float mag_x;
    float mag_y;
    float mag_z;
    uint32_t dt;
} ist8310_data_t;

enum ist8310_state_t {
    IST8310_RESET,
    IST8310_RESET_WAIT,
    IST8310_CONF,
    IST8310_MEAS,
    IST8310_READ
};

extern ist8310_data_t ist8310_data;
extern enum ist8310_state_t ist8310_state;

int IST8310_Init();
void IST8310_Run();

// TODO replace to another src
double IST8310_Get_magx();
double IST8310_Get_magy();
double IST8310_Get_magz();
int IST8310_MeasReady();
