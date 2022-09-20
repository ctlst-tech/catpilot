#include "ms5611.h"
#include "ms5611_reg.h"
#include "init.h"
#include "cfg.h"
#include "timer.h"

static char *device = "MS5611";

// Data structures
static ms5611_cfg_t ms5611_cfg;
static enum ms5611_state_t ms5611_state;
static ms5611_calib_t ms5611_calib;
static ms5611_data_t ms5611_data;
static ms5611_meas_t ms5611_meas;

// Private functions
static void MS5611_ChipSelection(void);
static void MS5611_ChipDeselection(void);
static void MS5611_ReadCalib(void);
static void MS5611_ReadMeas(void);
static void MS5611_TempProcess(void);
static void MS5611_PressProcess(void);

// Sync
static SemaphoreHandle_t measrdy_semaphore;

// Public functions
int MS5611_Init(spi_cfg_t *spi, gpio_cfg_t *cs) {
    if(spi == NULL || cs == NULL) return -1;

    ms5611_cfg.spi = spi;
    ms5611_cfg.cs = cs;

    if(measrdy_semaphore == NULL) measrdy_semaphore = xSemaphoreCreateBinary();

    xSemaphoreTake(measrdy_semaphore, 0);
    ms5611_state = MS5611_RESET;

    return 0;
}

void MS5611_Run(void) {
    switch(ms5611_state) {

    case MS5611_RESET:
        break;

    case MS5611_READ_CALIB:
        break;

    case MS5611_READ_MEAS:
        break;

    case MS5611_FAIL:
        break;
    }
}

int MS5611_Operation(void) {
    if(ms5611_state == MS5611_READ_MEAS) {
        return 0;
    } else {
        return -1;
    }
}

double MS5611_Get_T(void) {
    return ms5611_meas.T;
}

double MS5611_Get_P(void) {
    return ms5611_meas.P;
}

int MS5611_MeasReady(void) {
    xSemaphoreTake(measrdy_semaphore, portMAX_DELAY);
    return 1;
}

// Private functions
static void MS5611_ChipSelection(void) {
    GPIO_Reset(ms5611_cfg.cs);
}

static void MS5611_ChipDeselection(void) {
    GPIO_Set(ms5611_cfg.cs);
}

static void MS5611_ReadCalib(void) {

}

static void MS5611_ReadMeas(void) {

}

static void MS5611_TempProcess(void) {

}

static void MS5611_PressProcess(void) {
    
}
