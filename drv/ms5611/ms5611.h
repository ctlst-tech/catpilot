#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"
#include "const.h"

typedef struct {
    spi_t *spi;
    gpio_t *cs;
} ms5611_cfg_t;

enum ms5611_state_t {
    MS5611_RESET,
    MS5611_READ_CALIB,
    MS5611_READ_MEAS,
    MS5611_FAIL
};

int MS5611_Init(spi_t *spi, gpio_t *cs);
int MS5611_Operation(void);
void MS5611_Run(void);

double MS5611_Get_T(void);
double MS5611_Get_P(void);
int MS5611_MeasReady(void);
