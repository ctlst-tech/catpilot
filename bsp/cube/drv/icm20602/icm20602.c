#include "icm20602.h"
#include "icm20602_reg.h"
#include "init.h"
#include "cfg.h"

static char *device = "ICM20602";

icm20602_fifo_t icm20602_fifo;

enum icm20602_state_t icm20602_state;

uint8_t ICM20602_ReadReg(uint8_t reg);
void ICM20602_WriteReg(uint8_t reg, uint8_t value);
void ICM20602_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits);
int ICM20602_Configure();
void ICM20602_AccelConfigure();
void ICM20602_GyroConfigure();
int ICM20602_Probe();
void ICM20602_Statistics();
void ICM20602_FIFOCount();
int ICM20602_FIFORead();
void ICM20602_FIFOReset();
void ICM20602_AccelProcess();
void ICM20602_GyroProcess();
void ICM20602_TempProcess();

static gpio_cfg_t *icm20602_cs = &gpio_spi1_cs2;
static exti_cfg_t *icm20602_drdy = &exti_spi1_drdy2;
static SemaphoreHandle_t drdy_semaphore;

static SemaphoreHandle_t measrdy_semaphore;

static SemaphoreHandle_t timer_semaphore;
static uint32_t t0;

static uint32_t attempt = 0;

typedef struct {
    spi_cfg_t *spi;
    icm20602_param_t param;
} icm20602_cfg_t;
static icm20602_cfg_t icm20602_cfg;

static FIFOBuffer_t icm20602_FIFOBuffer;
static FIFOParam_t icm20602_FIFOParam;

static TickType_t icm20602_last_sample = 0;

int ICM20602_Init() {
    int rv = 0;

    icm20602_cfg.spi = &spi1;

    if(drdy_semaphore == NULL) drdy_semaphore = xSemaphoreCreateBinary();

    if(measrdy_semaphore == NULL) measrdy_semaphore = xSemaphoreCreateBinary();
    xSemaphoreTake(measrdy_semaphore, 0);

    if(timer_semaphore == NULL) timer_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(timer_semaphore);

    icm20602_state = ICM20602_RESET;

    return rv;
}

void ICM20602_ChipSelection() {
    GPIO_Reset(icm20602_cs);
}

void ICM20602_ChipDeselection() {
    GPIO_Set(icm20602_cs);
}

void ICM20602_Run() {
    switch(icm20602_state) {

    case ICM20602_RESET:
        if(xSemaphoreTake(timer_semaphore, 0)) {
            ICM20602_WriteReg(PWR_MGMT_1, DEVICE_RESET);
            t0 = xTaskGetTickCount();
        }
        if(xTaskGetTickCount() - t0 > 2.0) {
            icm20602_state = ICM20602_RESET_WAIT;
            xSemaphoreGive(timer_semaphore);
        }

        break;

    case ICM20602_RESET_WAIT:
        if(xSemaphoreTake(timer_semaphore, 0)) {
            if((ICM20602_ReadReg(WHO_AM_I) == WHOAMI)
                && (ICM20602_ReadReg(PWR_MGMT_1) == 0x41)
                && (ICM20602_ReadReg(CONFIG) == 0x80)) {

                    ICM20602_WriteReg(I2C_IF, I2C_IF_DIS);
                    ICM20602_WriteReg(PWR_MGMT_1, CLKSEL_0);
                    ICM20602_WriteReg(SIGNAL_PATH_RESET, ACCEL_RST | TEMP_RST);
                    ICM20602_SetClearReg(USER_CTRL, SIG_COND_RST, 0);

                    t0 = xTaskGetTickCount();
                } else {
                    LOG_ERROR(device, "Wrong default registers values after reset");
                    icm20602_state = ICM20602_RESET;
                    xSemaphoreGive(timer_semaphore);
                }
        }
        if(xTaskGetTickCount() - t0 > 100) {
            icm20602_state = ICM20602_CONF;
            LOG_DEBUG(device, "Device available");
            xSemaphoreGive(timer_semaphore);
        }
        break;

    case ICM20602_CONF:
        if(xTaskGetTickCount() - t0 > 100) {
            if(xSemaphoreTake(timer_semaphore, 0)) t0 = xTaskGetTickCount();
            if(ICM20602_Configure()) {
                icm20602_state = ICM20602_FIFO_READ;
                ICM20602_FIFOReset();
                icm20602_last_sample = xTaskGetTickCount();
                LOG_DEBUG(device, "Device configured");
                ICM20602_FIFOCount();
                ICM20602_FIFORead();
                xSemaphoreGive(timer_semaphore);
            } else {
                LOG_ERROR(device, "Failed configuration, retrying...");
                t0 = xTaskGetTickCount();
                attempt++;
                if(attempt > 5) {
                    icm20602_state = ICM20602_RESET;
                    LOG_ERROR(device, "Failed configuration, reset...");
                    attempt =  0;
                    xSemaphoreGive(timer_semaphore);
                }
            }
        }
        break;

    case ICM20602_FIFO_READ:
        if(xSemaphoreTake(drdy_semaphore, portMAX_DELAY)) {
            xSemaphoreGive(measrdy_semaphore);
            ICM20602_FIFOCount();
            ICM20602_FIFORead();
            icm20602_fifo.dt = xTaskGetTickCount() - icm20602_last_sample;
            icm20602_fifo.samples = icm20602_FIFOParam.samples;
            icm20602_last_sample = xTaskGetTickCount();
        }
        break;
    }
}

// TODO replace to another src
// TODO add processing
double ICM20602_Get_ax() {
    return (icm20602_fifo.accel_x[0]);
}

double ICM20602_Get_ay() {
    return (icm20602_fifo.accel_y[0]);
}

double ICM20602_Get_az() {
    return (icm20602_fifo.accel_z[0]);
}

double ICM20602_Get_wx() {
    return (icm20602_fifo.gyro_x[0]);
}

double ICM20602_Get_wy() {
    return (icm20602_fifo.gyro_y[0]);
}

double ICM20602_Get_wz() {
    return (icm20602_fifo.gyro_z[0]);
}

int ICM20602_MeasReady() {
    xSemaphoreTake(measrdy_semaphore, portMAX_DELAY);
    return 1;
}

uint8_t ICM20602_ReadReg(uint8_t reg) {
    uint8_t cmd = reg | READ;
    uint8_t data;

    ICM20602_ChipSelection();
    SPI_Transmit(icm20602_cfg.spi, &cmd, 1);
    SPI_Receive(icm20602_cfg.spi, &data, 1);
    ICM20602_ChipDeselection();

    return data;
}

void ICM20602_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = value;

    ICM20602_ChipSelection();
    SPI_Transmit(icm20602_cfg.spi, data, 2);
    ICM20602_ChipDeselection();
}

void ICM20602_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits) {
    uint8_t orig_val = ICM20602_ReadReg(reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        ICM20602_WriteReg(reg, val);
    }
}

int ICM20602_Configure() {
    uint8_t orig_val;
    int rv = 1;

    // Set configure
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        ICM20602_SetClearReg(reg_cfg[i].reg, reg_cfg[i].setbits, reg_cfg[i].clearbits);
    }

    // Check
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        orig_val = ICM20602_ReadReg(reg_cfg[i].reg);

        if((orig_val & reg_cfg[i].setbits) != reg_cfg[i].setbits) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not set)",
            (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].setbits);
            rv = 0;
        }

        if((orig_val & reg_cfg[i].clearbits) != 0) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not cleared)",
            (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    // Set scale and range for processing
    ICM20602_AccelConfigure();
    ICM20602_GyroConfigure();

    // Enable EXTI IRQ for DataReady pin
    EXTI_EnableIRQ(icm20602_drdy);

    return rv;
}

void ICM20602_AccelConfigure() {

    const uint8_t ACCEL_FS_SEL = ICM20602_ReadReg(ACCEL_CONFIG) & (BIT4 | BIT3);

    if(ACCEL_FS_SEL == ACCEL_FS_SEL_2G) {
        icm20602_cfg.param.accel_scale = (CONST_G / 16384.f);
        icm20602_cfg.param.accel_range = (2.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_4G) {
        icm20602_cfg.param.accel_scale = (CONST_G / 8192.f);
        icm20602_cfg.param.accel_range = (4.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_8G) {
        icm20602_cfg.param.accel_scale = (CONST_G / 4096.f);
        icm20602_cfg.param.accel_range = (8.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_16G) {
        icm20602_cfg.param.accel_scale = (CONST_G / 2048.f);
        icm20602_cfg.param.accel_range = (16.f * CONST_G);
    }
}

void ICM20602_GyroConfigure() {

    const uint8_t FS_SEL = ICM20602_ReadReg(GYRO_CONFIG) & (BIT4 | BIT3);

    if(FS_SEL == FS_SEL_250_DPS) {
        icm20602_cfg.param.gyro_range = 250.f;
    } else if(FS_SEL == FS_SEL_500_DPS) {
        icm20602_cfg.param.gyro_range = 500.f;
    } else if(FS_SEL == FS_SEL_1000_DPS) {
        icm20602_cfg.param.gyro_range = 1000.f;
    } else if(FS_SEL == FS_SEL_2000_DPS) {
        icm20602_cfg.param.gyro_range = 2000.f;
    }

    icm20602_cfg.param.gyro_scale = (icm20602_cfg.param.gyro_range / 32768.f);
}

void ICM20602_FIFOCount() {
    uint8_t cmd = FIFO_COUNTH | READ;
    uint8_t data[2];

    ICM20602_ChipSelection();
    SPI_Transmit(icm20602_cfg.spi, &cmd, 1);
    SPI_Receive(icm20602_cfg.spi, data, 2);
    ICM20602_ChipDeselection();

    icm20602_FIFOParam.samples = msblsb16(data[0], data[1]);
    icm20602_FIFOParam.bytes = icm20602_FIFOParam.samples * sizeof(FIFO_t);
}

int ICM20602_FIFORead() {
    uint8_t cmd = FIFO_COUNTH | READ;

    ICM20602_ChipSelection();
    SPI_Transmit(icm20602_cfg.spi, &cmd, 1);
    SPI_Receive(icm20602_cfg.spi, (uint8_t *)&icm20602_FIFOBuffer,
                icm20602_FIFOParam.bytes);
    ICM20602_ChipDeselection();

    ICM20602_TempProcess();
    ICM20602_AccelProcess();
    ICM20602_GyroProcess();

    EXTI_EnableIRQ(icm20602_drdy);

    return 0;
}

void ICM20602_FIFOReset() {
    ICM20602_WriteReg(FIFO_EN, 0);
    ICM20602_SetClearReg(USER_CTRL, USR_CTRL_FIFO_RST, USR_CTRL_FIFO_EN);

    for(int i = 0; i < SIZE_REG_CFG; i++) {
        if(reg_cfg[i].reg == FIFO_EN || reg_cfg[i].reg == USER_CTRL) {
            ICM20602_SetClearReg(reg_cfg[i].reg, reg_cfg[i].setbits, reg_cfg[i].clearbits);
        }
    }
}

void ICM20602_AccelProcess() {
    for (int i = 0; i < icm20602_FIFOParam.samples; i++) {
        int16_t accel_x = msblsb16(icm20602_FIFOBuffer.buf[i].ACCEL_XOUT_H,
                                    icm20602_FIFOBuffer.buf[i].ACCEL_XOUT_L);
        int16_t accel_y = msblsb16(icm20602_FIFOBuffer.buf[i].ACCEL_YOUT_H,
                                    icm20602_FIFOBuffer.buf[i].ACCEL_YOUT_L);
        int16_t accel_z = msblsb16(icm20602_FIFOBuffer.buf[i].ACCEL_ZOUT_H,
                                    icm20602_FIFOBuffer.buf[i].ACCEL_ZOUT_L);

        icm20602_fifo.accel_x[i] = accel_x * icm20602_cfg.param.accel_scale;
        icm20602_fifo.accel_y[i] = ((accel_y == INT16_MIN) ? INT16_MAX : -accel_y) *
                                        icm20602_cfg.param.accel_scale;
        icm20602_fifo.accel_z[i] = ((accel_z == INT16_MIN) ? INT16_MAX : -accel_z) *
                                        icm20602_cfg.param.accel_scale;
    }
}

void ICM20602_GyroProcess() {
    for (int i = 0; i < icm20602_FIFOParam.samples; i++) {
        int16_t gyro_x = msblsb16(icm20602_FIFOBuffer.buf[i].GYRO_XOUT_H,
                                    icm20602_FIFOBuffer.buf[i].GYRO_XOUT_L);
        int16_t gyro_y = msblsb16(icm20602_FIFOBuffer.buf[i].GYRO_YOUT_H,
                                    icm20602_FIFOBuffer.buf[i].GYRO_YOUT_L);
        int16_t gyro_z = msblsb16(icm20602_FIFOBuffer.buf[i].GYRO_ZOUT_H,
                                    icm20602_FIFOBuffer.buf[i].GYRO_ZOUT_L);

        icm20602_fifo.gyro_x[i] = gyro_x * icm20602_cfg.param.gyro_scale;
        icm20602_fifo.gyro_y[i] = ((gyro_y == INT16_MIN) ? INT16_MAX : -gyro_y) *
                                    icm20602_cfg.param.gyro_scale;
        icm20602_fifo.gyro_z[i] = ((gyro_z == INT16_MIN) ? INT16_MAX : -gyro_z) *
                                    icm20602_cfg.param.gyro_scale;
    }
}

void ICM20602_TempProcess() {
    float temperature_sum = 0;

    for (int i = 0; i < icm20602_FIFOParam.samples; i++) {
        const int16_t t = msblsb16(icm20602_FIFOBuffer.buf[i].TEMP_H,
                                    icm20602_FIFOBuffer.buf[i].TEMP_L);
        temperature_sum += t;
    }

    const float temperature_avg = temperature_sum / icm20602_FIFOParam.samples;
    const float temperature_C = (temperature_avg / TEMP_SENS) + TEMP_OFFSET;

    icm20602_fifo.temp = temperature_C;
}

int ICM20602_Probe() {
    uint8_t whoami;
    whoami = ICM20602_ReadReg(WHO_AM_I);
    if(whoami != WHOAMI) {
        LOG_ERROR(device, "unexpected WHO_AM_I reg 0x%02x\n", whoami);
        return ENODEV;
    }
    return 0;
}

void ICM20602_Statistics() {
    LOG_DEBUG(device, "Statistics:");
    LOG_DEBUG(device, "accel_x = %.3f [m/s2]", icm20602_fifo.accel_x[0]);
    LOG_DEBUG(device, "accel_y = %.3f [m/s2]", icm20602_fifo.accel_y[0]);
    LOG_DEBUG(device, "accel_z = %.3f [m/s2]", icm20602_fifo.accel_z[0]);
    LOG_DEBUG(device, "gyro_x  = %.3f [deg/s]", icm20602_fifo.gyro_x[0]);
    LOG_DEBUG(device, "gyro_y  = %.3f [deg/s]", icm20602_fifo.gyro_y[0]);
    LOG_DEBUG(device, "gyro_z  = %.3f [deg/s]", icm20602_fifo.gyro_z[0]);
    LOG_DEBUG(device, "temp    = %.3f [C]", icm20602_fifo.temp);
    LOG_DEBUG(device, "N       = %lu [samples]", icm20602_fifo.samples);
    LOG_DEBUG(device, "dt      = %lu [ms]", icm20602_fifo.dt);
    LOG_DEBUG(device, "N = %lu", icm20602_fifo.samples);
}

void ICM20602_DataReadyHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(drdy_semaphore, &xHigherPriorityTaskWoken);

    HAL_EXTI_ClearPending((EXTI_HandleTypeDef *)&icm20602_drdy->EXTI_Handle,
                            EXTI_TRIGGER_RISING);
    EXTI_DisableIRQ(icm20602_drdy);

    if(xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
