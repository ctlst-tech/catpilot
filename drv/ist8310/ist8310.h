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

typedef struct {
    i2c_t *i2c;
    ist8310_param_t param;
} ist8310_cfg_t;

enum ist8310_state_t {
    IST8310_RESET,
    IST8310_RESET_WAIT,
    IST8310_CONF,
    IST8310_MEAS,
    IST8310_READ,
    IST8310_FAIL
};

int IST8310_Init(i2c_t *i2c);
int IST8310_Operation(void);
void IST8310_Run(void);

double IST8310_Get_magx(void);
double IST8310_Get_magy(void);
double IST8310_Get_magz(void);
int IST8310_MeasReady(void);
