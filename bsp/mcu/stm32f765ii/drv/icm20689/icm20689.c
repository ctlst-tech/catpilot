#include "icm20689.h"
#include "icm20689_reg.h"
#include "board.h"
#include "board_cfg.h"

static char *device = "ICM20689";

icm20689_fifo_t icm20689_fifo;

enum icm20689_state_t {
    icm20689_RESET,
    icm20689_RESET_WAIT,
    icm20689_CONF,
    icm20689_FIFO_READ
};
enum icm20689_state_t icm20689_state;

uint8_t icm20689_ReadReg(uint8_t reg);
void icm20689_WriteReg(uint8_t reg, uint8_t value);
void icm20689_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits);
int icm20689_Configure();
void icm20689_AccelConfigure();
void icm20689_GyroConfigure();
int icm20689_Probe();
void icm20689_Statistics();
void icm20689_FIFOCount();
int icm20689_FIFORead();
void icm20689_FIFOReset();
void icm20689_AccelProcess();
void icm20689_GyroProcess();
void icm20689_TempProcess();

static gpio_cfg_t icm20689_cs = GPIO_SPI1_CS2;
static exti_cfg_t icm20689_drdy = EXTI_SPI1_DRDY2;
static SemaphoreHandle_t drdy_semaphore;

typedef struct {
    spi_cfg_t *spi;
    icm20689_param_t param;
} icm20689_cfg_t;
static icm20689_cfg_t icm20689_cfg;

static FIFOBuffer_t icm20689_FIFOBuffer;
static FIFOParam_t icm20689_FIFOParam;

static TickType_t icm20689_last_sample = 0;

int icm20689_Init() {
    int rv = 0;

    icm20689_cfg.spi = &spi1;

    if(drdy_semaphore == NULL) drdy_semaphore = xSemaphoreCreateBinary();

    rv |= EXTI_Init(&icm20689_drdy);
    rv |= GPIO_Init(&icm20689_cs);

    icm20689_state = icm20689_RESET;

    return rv;
}

void icm20689_ChipSelection() {
    GPIO_Reset(&icm20689_cs);
}

void icm20689_ChipDeselection() {
    GPIO_Set(&icm20689_cs);
}

void icm20689_Run() {
    switch(icm20689_state) {

    case icm20689_RESET:
        icm20689_WriteReg(PWR_MGMT_1, DEVICE_RESET);
        icm20689_state = icm20689_RESET_WAIT;
        vTaskDelay(2);
        break;

    case icm20689_RESET_WAIT:
        if ((icm20689_ReadReg(WHO_AM_I) == WHOAMI)
            && (icm20689_ReadReg(PWR_MGMT_1) == 0x40)) {
                icm20689_WriteReg(PWR_MGMT_1, CLKSEL_0);
                icm20689_WriteReg(SIGNAL_PATH_RESET, ACCEL_RST | TEMP_RST);
                icm20689_SetClearReg(USER_CTRL, SIG_COND_RST | I2C_IF_DIS, 0);
                icm20689_state = icm20689_CONF;
                vTaskDelay(1000);
            } else {
                printf("%s: Wrong default registers values after reset\n", device);
                vTaskDelay(1000);
            }
        break;

    case icm20689_CONF:
        if(icm20689_Configure()) {
            icm20689_state = icm20689_FIFO_READ;
            icm20689_FIFOReset();
            icm20689_last_sample = xTaskGetTickCount();
        } else {
            printf("%s: Wrong configuration, reset\n", device);
            icm20689_state = icm20689_RESET;
            vTaskDelay(1000);
        }
        break;

    case icm20689_FIFO_READ:
        if(xSemaphoreTake(drdy_semaphore, portMAX_DELAY)) {
            icm20689_FIFOCount();
            icm20689_FIFORead();
            icm20689_fifo.dt = xTaskGetTickCount() - icm20689_last_sample;
            icm20689_fifo.samples = icm20689_FIFOParam.samples;
            icm20689_last_sample = xTaskGetTickCount();
        }
        break;
    }
}

uint8_t icm20689_ReadReg(uint8_t reg) {
    uint8_t cmd = reg | READ;
    uint8_t data;

    icm20689_ChipSelection();
    SPI_Transmit(icm20689_cfg.spi, &cmd, 1);
    SPI_Receive(icm20689_cfg.spi, &data, 1);
    icm20689_ChipDeselection();

    return data;
}

void icm20689_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = value;

    icm20689_ChipSelection();
    SPI_Transmit(icm20689_cfg.spi, data, 2);
    icm20689_ChipDeselection();
}

void icm20689_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits) {
    uint8_t orig_val = icm20689_ReadReg(reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        icm20689_WriteReg(reg, val);
    }
}

int icm20689_Configure() {
    uint8_t orig_val;
    int rv = 1;

    // Set configure
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        icm20689_SetClearReg(reg_cfg[i].reg, reg_cfg[i].setbits, reg_cfg[i].clearbits);
    }

    // Check
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        orig_val = icm20689_ReadReg(reg_cfg[i].reg);

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

    // Set scale and range for processing
    icm20689_AccelConfigure();
    icm20689_GyroConfigure();

    // Enable EXTI IRQ for DataReady pin
    EXTI_EnableIRQ(&icm20689_drdy);

    return rv;
}

void icm20689_AccelConfigure() {

    const uint8_t ACCEL_FS_SEL = icm20689_ReadReg(ACCEL_CONFIG) & (BIT4 | BIT3);

    if(ACCEL_FS_SEL == ACCEL_FS_SEL_2G) {
        icm20689_cfg.param.accel_scale = (CONST_G / 16384.f);
        icm20689_cfg.param.accel_range = (2.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_4G) {
        icm20689_cfg.param.accel_scale = (CONST_G / 8192.f);
        icm20689_cfg.param.accel_range = (4.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_8G) {
        icm20689_cfg.param.accel_scale = (CONST_G / 4096.f);
        icm20689_cfg.param.accel_range = (8.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_16G) {
        icm20689_cfg.param.accel_scale = (CONST_G / 2048.f);
        icm20689_cfg.param.accel_range = (16.f * CONST_G);
    }
}

void icm20689_GyroConfigure() {

    const uint8_t FS_SEL = icm20689_ReadReg(GYRO_CONFIG) & (BIT4 | BIT3);

    if(FS_SEL == FS_SEL_250_DPS) {
        icm20689_cfg.param.gyro_range = 250.f;
    } else if(FS_SEL == FS_SEL_500_DPS) {
        icm20689_cfg.param.gyro_range = 500.f;
    } else if(FS_SEL == FS_SEL_1000_DPS) {
        icm20689_cfg.param.gyro_range = 1000.f;
    } else if(FS_SEL == FS_SEL_2000_DPS) {
        icm20689_cfg.param.gyro_range = 2000.f;
    }

    icm20689_cfg.param.gyro_scale = (icm20689_cfg.param.gyro_range / 32768.f);
}

void icm20689_FIFOCount() {
    uint8_t cmd = FIFO_COUNTH | READ;
    uint8_t data[2];

    icm20689_ChipSelection();
    SPI_Transmit(icm20689_cfg.spi, &cmd, 1);
    SPI_Receive(icm20689_cfg.spi, data, 2);
    icm20689_ChipDeselection();

    icm20689_FIFOParam.samples = msblsb16(data[0], data[1]);
    icm20689_FIFOParam.bytes = icm20689_FIFOParam.samples * sizeof(FIFO_t);
}

int icm20689_FIFORead() {
    uint8_t cmd = FIFO_COUNTH | READ;

    icm20689_ChipSelection();
    SPI_Transmit(icm20689_cfg.spi, &cmd, 1);
    SPI_Receive(icm20689_cfg.spi, (uint8_t *)&icm20689_FIFOBuffer,
                icm20689_FIFOParam.bytes);
    icm20689_ChipDeselection();

    icm20689_TempProcess();
    icm20689_AccelProcess();
    icm20689_GyroProcess();

    EXTI_EnableIRQ(&icm20689_drdy);

    return 0;
}

void icm20689_FIFOReset() {
    icm20689_WriteReg(FIFO_EN, 0);
    icm20689_SetClearReg(USER_CTRL, USR_CTRL_FIFO_RST, USR_CTRL_FIFO_EN);

    for(int i = 0; i < SIZE_REG_CFG; i++) {
        if(reg_cfg[i].reg == FIFO_EN || reg_cfg[i].reg == USER_CTRL) {
            icm20689_SetClearReg(reg_cfg[i].reg, reg_cfg[i].setbits, reg_cfg[i].clearbits);
        }
    }
}

void icm20689_AccelProcess() {
	for (int i = 0; i < icm20689_FIFOParam.samples; i++) {
		int16_t accel_x = msblsb16(icm20689_FIFOBuffer.buf[i].ACCEL_XOUT_H,
                                    icm20689_FIFOBuffer.buf[i].ACCEL_XOUT_L);
		int16_t accel_y = msblsb16(icm20689_FIFOBuffer.buf[i].ACCEL_YOUT_H,
                                    icm20689_FIFOBuffer.buf[i].ACCEL_YOUT_L);
		int16_t accel_z = msblsb16(icm20689_FIFOBuffer.buf[i].ACCEL_ZOUT_H,
                                    icm20689_FIFOBuffer.buf[i].ACCEL_ZOUT_L);

		icm20689_fifo.accel_x[i] = accel_x * icm20689_cfg.param.accel_scale;
		icm20689_fifo.accel_y[i] = ((accel_y == INT16_MIN) ? INT16_MAX : -accel_y) *
                                        icm20689_cfg.param.accel_scale;
		icm20689_fifo.accel_z[i] = ((accel_z == INT16_MIN) ? INT16_MAX : -accel_z) *
                                        icm20689_cfg.param.accel_scale;
	}
}

void icm20689_GyroProcess() {
	for (int i = 0; i < icm20689_FIFOParam.samples; i++) {
		int16_t gyro_x = msblsb16(icm20689_FIFOBuffer.buf[i].GYRO_XOUT_H,
                                    icm20689_FIFOBuffer.buf[i].GYRO_XOUT_L);
		int16_t gyro_y = msblsb16(icm20689_FIFOBuffer.buf[i].GYRO_YOUT_H,
                                    icm20689_FIFOBuffer.buf[i].GYRO_YOUT_L);
		int16_t gyro_z = msblsb16(icm20689_FIFOBuffer.buf[i].GYRO_ZOUT_H,
                                    icm20689_FIFOBuffer.buf[i].GYRO_ZOUT_L);

		icm20689_fifo.gyro_x[i] = gyro_x * icm20689_cfg.param.gyro_scale;
		icm20689_fifo.gyro_y[i] = ((gyro_y == INT16_MIN) ? INT16_MAX : -gyro_y) *
                                    icm20689_cfg.param.gyro_scale;
		icm20689_fifo.gyro_z[i] = ((gyro_z == INT16_MIN) ? INT16_MAX : -gyro_z) *
                                    icm20689_cfg.param.gyro_scale;
	}
}

void icm20689_TempProcess() {
	float temperature_sum = 0;

	for (int i = 0; i < icm20689_FIFOParam.samples; i++) {
		const int16_t t = msblsb16(icm20689_FIFOBuffer.buf[i].TEMP_H,
                                    icm20689_FIFOBuffer.buf[i].TEMP_L);
		temperature_sum += t;
	}

	const float temperature_avg = temperature_sum / icm20689_FIFOParam.samples;
    const float temperature_C = (temperature_avg / TEMP_SENS) + TEMP_OFFSET;

    icm20689_fifo.temp = temperature_C;
}

int icm20689_Probe() {
    uint8_t whoami;
    whoami = icm20689_ReadReg(WHO_AM_I);
    if(whoami != WHOAMI) {
        printf("%s: unexpected WHO_AM_I reg 0x%02x\n", device, whoami);
        return ENODEV;
    }
    return 0;
}

void icm20689_Statistics() {
    // TODO add time between FIFO reading
    printf("%s: Statistics:\n", device);
    printf("accel_x = %.3f [m/s2]\n", icm20689_fifo.accel_x[0]);
    printf("accel_y = %.3f [m/s2]\n", icm20689_fifo.accel_y[0]);
    printf("accel_z = %.3f [m/s2]\n", icm20689_fifo.accel_z[0]);
    printf("gyro_x  = %.3f [deg/s]\n", icm20689_fifo.gyro_x[0]);
    printf("gyro_y  = %.3f [deg/s]\n", icm20689_fifo.gyro_y[0]);
    printf("gyro_z  = %.3f [deg/s]\n", icm20689_fifo.gyro_z[0]);
    printf("temp    = %.3f [C]\n", icm20689_fifo.temp);
    printf("N       = %lu [samples]\n", icm20689_fifo.samples);
    printf("dt      = %lu [ms]\n", icm20689_fifo.dt);
}

void icm20689_DataReadyHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(drdy_semaphore, &xHigherPriorityTaskWoken);

    HAL_EXTI_ClearPending((EXTI_HandleTypeDef *)&icm20689_drdy.EXTI_Handle,
                            EXTI_TRIGGER_RISING);
    EXTI_DisableIRQ(&icm20689_drdy);

    if(xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
