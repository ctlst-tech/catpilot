#include "ms5611.h"
#include "ms5611_reg.h"
#include "init.h"
#include "cfg.h"
#include "timer.h"

static char *device = "MS5611";

// Data structures
static ms5611_cfg_t ms5611_cfg;
static enum ms5611_state_t ms5611_state;
static ms5611_prom_t ms5611_calib;
static ms5611_raw_t ms5611_raw;
static ms5611_meas_t ms5611_meas;

// Private functions
static void MS5611_ChipSelection(void);
static void MS5611_ChipDeselection(void);
static void MS5611_Reset(void);
uint16_t MS5611_ReadPROM(uint8_t word);
static int MS5611_ReadCalib(void);
static uint32_t MS5611_ReadADC(void);
static int MS5611_ReadMeas(void);
static void MS5611_ProcessMeas(void);

// Sync
static SemaphoreHandle_t measrdy_semaphore;
static int attempt;

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
        vTaskDelay(5);
        MS5611_Reset();
        vTaskDelay(5);
        ms5611_state = MS5611_READ_CALIB;
        break;

    case MS5611_READ_CALIB:
        if(MS5611_ReadCalib()) {
            LOG_ERROR(device, "Failed to read calibration values");
            attempt++;
        } else {
            vTaskDelay(10);
            MS5611_ChipSelection();
            SPI_Transmit(ms5611_cfg.spi, 
                         (uint8_t *)&CMD_MS5611_CONVERT_D1_OSR1024, 1);
            MS5611_ChipDeselection();
            ms5611_state = MS5611_READ_MEAS;
        }
        if(attempt > 5) {
            LOG_ERROR(device, "Fatal error");
            ms5611_state = MS5611_FAIL;
        }
        break;

    case MS5611_READ_MEAS:
        if(!MS5611_ReadMeas()) {
            MS5611_ProcessMeas();
        }
        break;

    case MS5611_FAIL:
        vTaskDelay(1000);
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
    SPI_ChipSelect(ms5611_cfg.spi, ms5611_cfg.cs);
}

static void MS5611_ChipDeselection(void) {
    SPI_ChipDeselect(ms5611_cfg.spi, ms5611_cfg.cs);
}

static void MS5611_Reset(void) {
    uint8_t reg = CMD_MS5611_RESET;

    MS5611_ChipSelection();
    SPI_Transmit(ms5611_cfg.spi, &reg, 1);
    MS5611_ChipDeselection();
}

uint16_t MS5611_ReadPROM(uint8_t word) {
    uint8_t data[3];
    data[0] = CMD_MS5611_PROM_ADDR + (word << 1);

    MS5611_ChipSelection();
    SPI_TransmitReceive(ms5611_cfg.spi, data, data, sizeof(data));
    MS5611_ChipDeselection();

    return (data[1] << 8 | data[2]);
}

static int MS5611_ReadCalib(void) {
    uint16_t prom[MS5611_PROM_SIZE] = {};

    for(int i = 0; i < MS5611_PROM_SIZE; i++) {
        prom[i] = MS5611_ReadPROM(i);
        if(prom[i] == 0 && i > 1) {
            return -1;
        }
    }

    uint16_t crc_get = prom[7] & 0xF;
    prom[7] &= 0xFF00;
    uint16_t crc_calc = crc4(prom);
    if(crc_get != crc_calc) {
        return -1;
    }

    ms5611_calib.C1 = prom[1];
    ms5611_calib.C2 = prom[2];
    ms5611_calib.C3 = prom[3];
    ms5611_calib.C4 = prom[4];
    ms5611_calib.C5 = prom[5];
    ms5611_calib.C6 = prom[6];

    return 0;
}

static uint32_t MS5611_ReadADC(void) {
    uint8_t data[4] = {};
    data[0] = CMD_MS5611_READ_ADC;

    MS5611_ChipSelection();
    SPI_TransmitReceive(ms5611_cfg.spi, data, data, sizeof(data));
    MS5611_ChipDeselection();

    uint32_t reg = (data[1] << 16) | (data[2] << 8) | data[3];

    return reg;
}

// Read 4 samples for pressure and 1 sample for temperature
static int MS5611_ReadMeas(void) {
    uint8_t cmd;
    static int ignore_next = 0;
    static int meas_previous_state = MS5611_READ_TEMP;
    static int meas_current_state = MS5611_READ_TEMP;

    // Read ADC after previous command
    uint32_t reg = MS5611_ReadADC();

    if(reg != 0 && ignore_next == 0) {
        meas_current_state++;
    }

    if(meas_current_state > MS5611_READ_PRES_4) {
        meas_current_state = MS5611_READ_TEMP;
    }

    switch(meas_current_state) {
    case(MS5611_READ_TEMP):
        cmd = CMD_MS5611_CONVERT_D1_OSR1024;
        break;
    case(MS5611_READ_PRES_1):
    case(MS5611_READ_PRES_2):
    case(MS5611_READ_PRES_3):
    case(MS5611_READ_PRES_4):
        cmd = CMD_MS5611_CONVERT_D2_OSR1024;
        break;
    }

    MS5611_ChipSelection();
    SPI_Transmit(ms5611_cfg.spi, &cmd, 1);
    MS5611_ChipDeselection();

    if(reg == 0 || reg == 0xFFFFFF) {
        ignore_next = 1;
        return -1;
    }

    if(ignore_next) {
        ignore_next = 0;
        return -1;
    }

    switch(meas_previous_state) {
    case(MS5611_READ_TEMP):
        ms5611_raw.D2 = reg;
        break;
    case(MS5611_READ_PRES_1):
    case(MS5611_READ_PRES_2):
    case(MS5611_READ_PRES_3):
    case(MS5611_READ_PRES_4):
        ms5611_raw.D1 = reg;
        break;
    }

    meas_previous_state = meas_current_state;

    return 0;
}

static void MS5611_ProcessMeas(void) {
    int32_t dT = 0, TEMP = 0, T2 = 0, P = 0;
    int64_t OFF = 0, SENS = 0, OFF2 = 0, SENS2 = 0;

    // Temperature
    dT = ms5611_raw.D2 - (uint32_t)(ms5611_calib.C5) * (1 << 8);
    TEMP = 2000 + dT * ms5611_calib.C6 / (1 << 23);

    // Pressure
    OFF = (uint32_t)ms5611_calib.C2 * (1 << 16) + 
          (uint32_t)(ms5611_calib.C4 * dT) / (1 << 7);

    SENS = (uint32_t)ms5611_calib.C1 * (1 << 15) + 
           (uint32_t)(ms5611_calib.C3 * dT) / (1 << 8);

    // Low temperature compensation
    if(TEMP < 2000) {
        T2 = (dT * dT) / (1 << 31);
        OFF2 = 5 * (TEMP - 2000) * (TEMP - 2000) / 2;
        SENS2 = 5 * (TEMP - 2000) * (TEMP - 2000) / 4;
        if(TEMP < -1500) {
            OFF2 = OFF2 + 7 * (TEMP + 1500) * (TEMP + 1500);
            SENS2 = SENS2 + 11 * (TEMP + 1500) * (TEMP + 1500) / 2;
        }
    }

    TEMP = TEMP - T2;
    OFF = OFF - OFF2;
    SENS = SENS - SENS2;

    P = (ms5611_raw.D1 * SENS / (1 << 21) - OFF) / (1 << 15);

    ms5611_meas.T = TEMP / 100.f;
    ms5611_meas.P = P;
}
