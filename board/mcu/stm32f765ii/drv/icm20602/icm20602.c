#include "icm20602.h"
#include "icm20602_reg.h"
#include "icm20602_conf.h"

static char *device = "ICM20602";

static SemaphoreHandle_t drdy_semaphore;

static gpio_cfg_t icm20602_mosi = GPIO_SPI1_MOSI;
static gpio_cfg_t icm20602_miso = GPIO_SPI1_MISO;
static gpio_cfg_t icm20602_sck  = GPIO_SPI1_SCK;
static gpio_cfg_t icm20602_cs   = GPIO_SPI1_CS2;

static exti_cfg_t icm20602_drdy = EXTI_SPI1_DRDY2;

// Others sensors on this SPI bus
// TODO move to driver sources
gpio_cfg_t cs1 = GPIO_SPI1_CS1;
gpio_cfg_t cs3 = GPIO_SPI1_CS3;
gpio_cfg_t cs4 = GPIO_SPI1_CS4;
// TODO move to driver sources

static dma_cfg_t dma_spi1_mosi;
static dma_cfg_t dma_spi1_miso;

static icm20602_cfg_t icm20602_cfg;

static FIFOBuffer_t FIFOBuffer;
static FIFOParam_t FIFOParam;

static TickType_t t_last = 0;

icm20602_fifo_t icm20602_fifo;

enum icm20602_state_t {
    ICM20602_RESET,
    ICM20602_RESET_WAIT,
    ICM20602_CONF,
    ICM20602_FIFO_READ
};

enum icm20602_state_t icm20602_state;

int ICM20602_Init() {
    int rv = 0;

    // Chip deselect
    GPIO_Init(&cs1);
    GPIO_Init(&cs3);
    GPIO_Init(&cs4);
    GPIO_Set(&cs1);
    GPIO_Set(&cs3);
    GPIO_Set(&cs4);

    icm20602_cfg.spi.SPI = SPI1;
    icm20602_cfg.spi.mosi_cfg = &icm20602_mosi;
    icm20602_cfg.spi.miso_cfg = &icm20602_miso;
    icm20602_cfg.spi.sck_cfg  = &icm20602_sck;
    icm20602_cfg.spi.cs_cfg   = &icm20602_cs;
    icm20602_cfg.spi.timeout  = ICM20602_TIMEOUT;
    icm20602_cfg.spi.priority = ICM20602_IRQ_PRIORITY;

    icm20602_cfg.spi.dma_miso_cfg = &dma_spi1_miso;
    icm20602_cfg.spi.dma_mosi_cfg = &dma_spi1_mosi;

    dma_spi1_mosi.DMA_InitStruct.Instance = DMA2_Stream3;
    dma_spi1_mosi.DMA_InitStruct.Init.Channel = DMA_CHANNEL_3;
    dma_spi1_mosi.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma_spi1_mosi.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_spi1_mosi.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_spi1_mosi.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_spi1_mosi.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_spi1_mosi.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    dma_spi1_mosi.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    dma_spi1_mosi.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_spi1_mosi.priority = ICM20602_IRQ_PRIORITY;

    dma_spi1_miso.DMA_InitStruct.Instance = DMA2_Stream0;
    dma_spi1_miso.DMA_InitStruct.Init.Channel = DMA_CHANNEL_3;
    dma_spi1_miso.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_spi1_miso.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_spi1_miso.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_spi1_miso.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_spi1_miso.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_spi1_miso.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    dma_spi1_miso.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    dma_spi1_miso.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_spi1_miso.priority = ICM20602_IRQ_PRIORITY;

    if(drdy_semaphore == NULL) drdy_semaphore = xSemaphoreCreateBinary();

    rv |= SPI_Init(&icm20602_cfg.spi);
    rv |= EXTI_Init(&icm20602_drdy);

    icm20602_state = ICM20602_RESET;

    return rv;
}

void ICM20602_ChipSelection() {
    GPIO_Reset(icm20602_cfg.spi.cs_cfg);
}

void ICM20602_ChipDeselection() {
    GPIO_Set(icm20602_cfg.spi.cs_cfg);
}

void ICM20602_Run() {
    switch(icm20602_state) {

    case ICM20602_RESET:
        ICM20602_WriteReg(PWR_MGMT_1, DEVICE_RESET);
        icm20602_state = ICM20602_RESET_WAIT;
        vTaskDelay(2);
        break;

    case ICM20602_RESET_WAIT:
        if ((ICM20602_ReadReg(WHO_AM_I) == WHOAMI)
            && (ICM20602_ReadReg(PWR_MGMT_1) == 0x41)
            && (ICM20602_ReadReg(CONFIG) == 0x80)) {
                ICM20602_WriteReg(I2C_IF, I2C_IF_DIS);
                ICM20602_WriteReg(PWR_MGMT_1, CLKSEL_0);
                ICM20602_WriteReg(SIGNAL_PATH_RESET, ACCEL_RST | TEMP_RST);
                ICM20602_SetClearReg(USER_CTRL, SIG_COND_RST, 0);
                icm20602_state = ICM20602_CONF;
                vTaskDelay(1000);
            } else {
                printf("%s: Wrong default registers values after reset\n", device);
                vTaskDelay(1000);
            }
        break;

    case ICM20602_CONF:
        if(ICM20602_Configure()) {
            icm20602_state = ICM20602_FIFO_READ;
            ICM20602_FIFOReset();
        } else {
            printf("%s: Wrong configuration, reset\n", device);
            icm20602_state = ICM20602_RESET;
            vTaskDelay(1000);
        }
        break;

    case ICM20602_FIFO_READ:
        if(xSemaphoreTake(drdy_semaphore, portMAX_DELAY)) {
            ICM20602_FIFOCount();
            ICM20602_FIFORead();
            icm20602_fifo.dt = xTaskGetTickCount() - t_last;
            icm20602_fifo.samples = FIFOParam.samples;
            t_last = xTaskGetTickCount();
        }
        break;
    }
}

uint8_t ICM20602_ReadReg(uint8_t reg) {
    uint8_t cmd = reg | READ;
    uint8_t data;

    ICM20602_ChipSelection();
    SPI_Transmit(&icm20602_cfg.spi, &cmd, 1);
    SPI_Receive(&icm20602_cfg.spi, &data, 1);
    ICM20602_ChipDeselection();

    return data;
}

void ICM20602_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = value;

    ICM20602_ChipSelection();
    SPI_Transmit(&icm20602_cfg.spi, data, 2);
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
    ICM20602_AccelConfigure();
    ICM20602_GyroConfigure();

    // Enable EXTI IRQ for DataReady pin
    EXTI_EnableIRQ(&icm20602_drdy);

    return rv;
}

void ICM20602_AccelConfigure() {

    const uint8_t ACCEL_FS_SEL = ICM20602_ReadReg(ACCEL_CONFIG) & (BIT4 | BIT3);

    if(ACCEL_FS_SEL == ACCEL_FS_SEL_2G) {
        icm20602_cfg.dim.accel_scale = (CONST_G / 16384.f);
        icm20602_cfg.dim.accel_range = (2.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_4G) {
        icm20602_cfg.dim.accel_scale = (CONST_G / 8192.f);
        icm20602_cfg.dim.accel_range = (4.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_8G) {
        icm20602_cfg.dim.accel_scale = (CONST_G / 4096.f);
        icm20602_cfg.dim.accel_range = (8.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_16G) {
        icm20602_cfg.dim.accel_scale = (CONST_G / 2048.f);
        icm20602_cfg.dim.accel_range = (16.f * CONST_G);
    }
}

void ICM20602_GyroConfigure() {

    const uint8_t FS_SEL = ICM20602_ReadReg(GYRO_CONFIG) & (BIT4 | BIT3);

    if(FS_SEL == FS_SEL_250_DPS) {
        icm20602_cfg.dim.gyro_range = 250.f;
    } else if(FS_SEL == FS_SEL_500_DPS) {
        icm20602_cfg.dim.gyro_range = 500.f;
    } else if(FS_SEL == FS_SEL_1000_DPS) {
        icm20602_cfg.dim.gyro_range = 1000.f;
    } else if(FS_SEL == FS_SEL_2000_DPS) {
        icm20602_cfg.dim.gyro_range = 2000.f;
    }

    icm20602_cfg.dim.gyro_scale = (icm20602_cfg.dim.gyro_range / 32768.f);
}

void ICM20602_FIFOCount() {
    uint8_t cmd = FIFO_COUNTH | READ;
    uint8_t data[2];

    ICM20602_ChipSelection();
    SPI_Transmit(&icm20602_cfg.spi, &cmd, 1);
    SPI_Receive(&icm20602_cfg.spi, data, 2);
    ICM20602_ChipDeselection();

    FIFOParam.samples = msblsb16(data[0], data[1]);
    FIFOParam.bytes = FIFOParam.samples * sizeof(FIFO_t);
}

int ICM20602_FIFORead() {
    uint8_t cmd = FIFO_COUNTH | READ;

    ICM20602_ChipSelection();
    SPI_Transmit(&icm20602_cfg.spi, &cmd, 1);
    SPI_Receive(&icm20602_cfg.spi, (uint8_t *)&FIFOBuffer, FIFOParam.bytes);
    ICM20602_ChipDeselection();

    ICM20602_TempProcess();
    ICM20602_AccelProcess();
    ICM20602_GyroProcess();

    EXTI_EnableIRQ(&icm20602_drdy);

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
	for (int i = 0; i < FIFOParam.samples; i++) {
		int16_t accel_x = msblsb16(FIFOBuffer.buf[i].ACCEL_XOUT_H, FIFOBuffer.buf[i].ACCEL_XOUT_L);
		int16_t accel_y = msblsb16(FIFOBuffer.buf[i].ACCEL_YOUT_H, FIFOBuffer.buf[i].ACCEL_YOUT_L);
		int16_t accel_z = msblsb16(FIFOBuffer.buf[i].ACCEL_ZOUT_H, FIFOBuffer.buf[i].ACCEL_ZOUT_L);

		icm20602_fifo.accel_x[i] = accel_x * icm20602_cfg.dim.accel_scale;
		icm20602_fifo.accel_y[i] = ((accel_y == INT16_MIN) ? INT16_MAX : -accel_y) * icm20602_cfg.dim.accel_scale;
		icm20602_fifo.accel_z[i] = ((accel_z == INT16_MIN) ? INT16_MAX : -accel_z) * icm20602_cfg.dim.accel_scale;
	}
}

void ICM20602_GyroProcess() {
	for (int i = 0; i < FIFOParam.samples; i++) {
		int16_t gyro_x = msblsb16(FIFOBuffer.buf[i].GYRO_XOUT_H, FIFOBuffer.buf[i].GYRO_XOUT_L);
		int16_t gyro_y = msblsb16(FIFOBuffer.buf[i].GYRO_YOUT_H, FIFOBuffer.buf[i].GYRO_YOUT_L);
		int16_t gyro_z = msblsb16(FIFOBuffer.buf[i].GYRO_ZOUT_H, FIFOBuffer.buf[i].GYRO_ZOUT_L);

		icm20602_fifo.gyro_x[i] = gyro_x * icm20602_cfg.dim.gyro_scale;
		icm20602_fifo.gyro_y[i] = ((gyro_y == INT16_MIN) ? INT16_MAX : -gyro_y) * icm20602_cfg.dim.gyro_scale;
		icm20602_fifo.gyro_z[i] = ((gyro_z == INT16_MIN) ? INT16_MAX : -gyro_z) * icm20602_cfg.dim.gyro_scale;
	}
}

void ICM20602_TempProcess() {
	float temperature_sum = 0;

	for (int i = 0; i < FIFOParam.samples; i++) {
		const int16_t t = msblsb16(FIFOBuffer.buf[i].TEMP_H, FIFOBuffer.buf[i].TEMP_L);
		temperature_sum += t;
	}

	const float temperature_avg = temperature_sum / FIFOParam.samples;
    const float temperature_C = (temperature_avg / TEMP_SENS) + TEMP_OFFSET;

    icm20602_fifo.temp = temperature_C;
}

int ICM20602_Probe() {
    uint8_t whoami;
    whoami = ICM20602_ReadReg(WHO_AM_I);
    if(whoami != WHOAMI) {
        printf("%s: unexpected WHO_AM_I reg 0x%02x\n", device, whoami);
        return ENODEV;
    }
    return 0;
}

void ICM20602_Statistics() {
    // TODO add time between FIFO reading
    printf("%s: Statistics:\n", device);
    printf("accel_x = %.3f [m/s2]\n", icm20602_fifo.accel_x[0]);
    printf("accel_y = %.3f [m/s2]\n", icm20602_fifo.accel_y[0]);
    printf("accel_z = %.3f [m/s2]\n", icm20602_fifo.accel_z[0]);
    printf("gyro_x  = %.3f [deg/s]\n", icm20602_fifo.gyro_x[0]);
    printf("gyro_y  = %.3f [deg/s]\n", icm20602_fifo.gyro_y[0]);
    printf("gyro_z  = %.3f [deg/s]\n", icm20602_fifo.gyro_z[0]);
    printf("temp    = %.3f [C]\n", icm20602_fifo.temp);
    printf("N       = %lu [samples]\n", icm20602_fifo.samples);
    printf("dt      = %lu [ms]\n", icm20602_fifo.dt);
}

void ICM20602_DataReadyHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(drdy_semaphore, &xHigherPriorityTaskWoken);

    HAL_EXTI_ClearPending((EXTI_HandleTypeDef *)&icm20602_drdy.EXTI_Handle, EXTI_TRIGGER_RISING);
    EXTI_DisableIRQ(&icm20602_drdy);

    if(xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void SPI1_IRQHandler(void) {
    SPI_IT_Handler(&icm20602_cfg.spi);
}

void DMA2_Stream3_IRQHandler(void) {
    SPI_DMA_MOSI_Handler(&icm20602_cfg.spi);
}

void DMA2_Stream0_IRQHandler(void) {
    SPI_DMA_MISO_Handler(&icm20602_cfg.spi);
}

void EXTI9_5_IRQHandler(void) {
    uint32_t line;

    line = EXTI->PR;

    switch(line) {
        case GPIO_PIN_5:
            ICM20602_DataReadyHandler();
            break;
        default:
            break;
    }
}
