#include "ist8310.h"
#include "ist8310_reg.h"
#include "init.h"
#include "cfg.h"

static char *device = "IST8310";

// Data structures
static ist8310_data_t ist8310_data;
static ist8310_cfg_t ist8310_cfg;
static enum ist8310_state_t ist8310_state;
static buffer_t buffer;

// Private functions
static uint8_t IST8310_ReadReg(uint8_t reg);
static void IST8310_WriteReg(uint8_t reg, uint8_t value);
static void IST8310_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits);
static int IST8310_Configure(void);
static void IST8310_Meas(void);
static int IST8310_Process(void);
static int IST8310_Probe(void);

static SemaphoreHandle_t measrdy_semaphore;
static SemaphoreHandle_t timer_semaphore;
static uint32_t attempt = 0;
static uint32_t t0;

// Public functions
int IST8310_Init(i2c_t *i2c) {
    if(i2c == NULL) return -1;

    ist8310_cfg.i2c = i2c;

    if(timer_semaphore == NULL) timer_semaphore = xSemaphoreCreateBinary();
    if(measrdy_semaphore == NULL) measrdy_semaphore = xSemaphoreCreateBinary();

    xSemaphoreTake(measrdy_semaphore, 0);
    xSemaphoreGive(timer_semaphore);

    ist8310_state = IST8310_RESET;

    return 0;
}

void IST8310_Run(void) {
    switch(ist8310_state) {

    case IST8310_RESET:
        if(xSemaphoreTake(timer_semaphore, 0)) {
            IST8310_WriteReg(CNTL2, SRST);
            t0 = xTaskGetTickCount();
        }
        if(xTaskGetTickCount() - t0 > 20.0) {
            ist8310_state = IST8310_RESET_WAIT;
            xSemaphoreGive(timer_semaphore);
        }
        break;

    case IST8310_RESET_WAIT:
        if(xSemaphoreTake(timer_semaphore, 0)) {
            IST8310_ReadReg(CNTL2);
            if ((IST8310_ReadReg(WHO_AM_I) == DEVICE_ID)
                && ((IST8310_ReadReg(CNTL2) & SRST) == 0)) {
                    t0 = xTaskGetTickCount();
                } else {
                    LOG_ERROR(device, "Wrong default registers values after reset");
                    ist8310_state = IST8310_RESET;
                    xSemaphoreGive(timer_semaphore);
                    attempt++;
                    if(attempt > 5) {
                        ist8310_state = IST8310_FAIL;
                        LOG_ERROR(device, "Fatal error");
                        attempt = 0;
                    }
                }
        }
        if(xTaskGetTickCount() - t0 > 10.0) {
            ist8310_state = IST8310_CONF;
            LOG_DEBUG(device, "Device available");
            xSemaphoreGive(timer_semaphore);
        }
        break;

    case IST8310_CONF:
        if(IST8310_Configure()) {
            ist8310_state = IST8310_MEAS;
            LOG_DEBUG(device, "Device configured");
        } else {
            LOG_ERROR(device, "Wrong configuration, reset");
            ist8310_state = IST8310_RESET;
        }
        break;

    case IST8310_MEAS:
        IST8310_WriteReg(CNTL1, SINGLE_MEAS);
        t0 = xTaskGetTickCount();
        ist8310_state = IST8310_READ;
        break;

    case IST8310_READ:
        if((xTaskGetTickCount() - t0) >= 20.0) {
            IST8310_Meas();
            IST8310_Process();
            IST8310_WriteReg(CNTL1, SINGLE_MEAS);
            t0 = xTaskGetTickCount();
            ist8310_data.dt = xTaskGetTickCount() - t0;
            xSemaphoreGive(measrdy_semaphore);
        }
        break;
    
    case IST8310_FAIL:
        vTaskDelay(1000);
        break;
    }
}

int IST8310_Operation(void) {
    if(ist8310_state == IST8310_READ) {
        return 0;
    } else {
        return -1;
    } 
}

double IST8310_Get_magx(void) {
    return (ist8310_data.mag_x);
}

double IST8310_Get_magy(void) {
    return (ist8310_data.mag_y);
}

double IST8310_Get_magz(void) {
    return (ist8310_data.mag_z);
}

int IST8310_MeasReady(void) {
    xSemaphoreTake(measrdy_semaphore, portMAX_DELAY);
    return 1;
}

// Private functions
static uint8_t IST8310_ReadReg(uint8_t reg) {
    uint8_t buf[2];

    buf[0] = (ADDRESS << 1) | READ;
    buf[1] = reg;

    I2C_Transmit(ist8310_cfg.i2c, buf[0], &buf[1], 1);
    I2C_Receive(ist8310_cfg.i2c, buf[0], &buf[1], 1);

    return buf[1];
}

static void IST8310_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t buf[3];

    buf[0] = (ADDRESS << 1) | WRITE;
    buf[1] = reg;
    buf[2] = value;

    I2C_Transmit(ist8310_cfg.i2c, buf[0], &buf[1], 2);
}

static void IST8310_Meas(void) {
    uint8_t buf[3];

    buf[0] = (ADDRESS << 1) | READ;
    buf[1] = STAT1;

    I2C_Transmit(ist8310_cfg.i2c, buf[0], &buf[1], 1);
    I2C_Receive(ist8310_cfg.i2c, buf[0], (uint8_t *)&buffer, sizeof(buffer_t));
}

static int IST8310_Process(void) {
    if(buffer.STAT1 & DRDY) {
        ist8310_data.mag_x = msblsb16(buffer.DATAXH, buffer.DATAXL) *
                                        ist8310_cfg.param.mag_scale;
        ist8310_data.mag_y = msblsb16(buffer.DATAYH, buffer.DATAYL) *
                                        ist8310_cfg.param.mag_scale;
        ist8310_data.mag_z = msblsb16(buffer.DATAZH, buffer.DATAZL) *
                                        ist8310_cfg.param.mag_scale;
        return 0;
    } else {
        return EPROTO;
    }
}

static void IST8310_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits) {
    uint8_t orig_val = IST8310_ReadReg(reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        IST8310_WriteReg(reg, val);
    }
}

static int IST8310_Configure(void) {
    uint8_t orig_val;
    int rv = 1;

    // Set configure
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        IST8310_SetClearReg(reg_cfg[i].reg, reg_cfg[i].setbits, reg_cfg[i].clearbits);
    }

    // Check
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        orig_val = IST8310_ReadReg(reg_cfg[i].reg);

        if((orig_val & reg_cfg[i].setbits) != reg_cfg[i].setbits) {
            printf("%s: 0x%02x: 0x%02x (0x%02x not set)\n", device,
            (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].setbits);
            rv = 0;
        }

        if((orig_val & reg_cfg[i].clearbits) != 0) {
            printf("%s: 0x%02x: 0x%02x (0x%02x not cleared)\n", device,
            (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    ist8310_cfg.param.mag_scale = (1.f / 1320.f); // 1320 LSB/Gauss

    return rv;
}

static int IST8310_Probe(void) {
    uint8_t whoami;
    whoami = IST8310_ReadReg(WHO_AM_I);
    if(whoami != DEVICE_ID) {
        printf("%s: Unexpected WHO_AM_I reg 0x%02x\n", device, whoami);
        return ENODEV;
    }
    return 0;
}
