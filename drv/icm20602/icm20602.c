#include "icm20602.h"
#include "icm20602_reg.h"

gpio_cfg_t icm20602_mosi = GPIO_SPI1_MOSI;
gpio_cfg_t icm20602_miso = GPIO_SPI1_MISO;
gpio_cfg_t icm20602_sck  = GPIO_SPI1_SCK;
gpio_cfg_t icm20602_cs   = GPIO_SPI1_CS2;
exti_cfg_t icm20602_drdy = GPIO_SPI1_DRDY2;

// Others sensors on this SPI bus
// TODO move to driver sources
gpio_cfg_t cs1 = GPIO_SPI1_CS1;
gpio_cfg_t cs3 = GPIO_SPI1_CS3;
gpio_cfg_t cs4 = GPIO_SPI1_CS4;
// TODO move to driver sources

dma_cfg_t dma_spi1_mosi;
dma_cfg_t dma_spi1_miso;

icm20602_cfg_t icm20602_cfg;
FIFOBuffer_t icm_20602_FIFO;

enum state_t {
    ICM20602_RESET,
    ICM20602_RESET_WAIT,
    ICM20602_CONF,
    ICM20602_FIFO_READ
};

enum state_t state;

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
    icm20602_cfg.spi.timeout  = 20;
    icm20602_cfg.spi.priority = 6;

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
    dma_spi1_mosi.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    dma_spi1_mosi.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_spi1_mosi.priority = 15;

    dma_spi1_miso.DMA_InitStruct.Instance = DMA2_Stream0;
    dma_spi1_miso.DMA_InitStruct.Init.Channel = DMA_CHANNEL_3;
    dma_spi1_miso.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_spi1_miso.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_spi1_miso.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_spi1_miso.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_spi1_miso.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_spi1_miso.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    dma_spi1_miso.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW; 
    dma_spi1_miso.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_spi1_miso.priority = 15;

    rv |= SPI_Init(&icm20602_cfg.spi);
    rv |= EXTI_Init(&icm20602_drdy);
    
    state = ICM20602_RESET;

    return rv;
}

void ICM20602_Run() {
    switch(state) {

        case ICM20602_RESET:
            ICM20602_WriteReg(PWR_MGMT_1, DEVICE_RESET);
            state = ICM20602_RESET_WAIT;
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
                    state = ICM20602_CONF;
                    vTaskDelay(1000);
                } else {
                    vTaskDelay(1000);
                }
            break;

        case ICM20602_CONF:
            if(ICM20602_Configure()) {
                state = ICM20602_FIFO_READ;
                ICM20602_FIFOReset();
            } else {
                state = ICM20602_RESET;
                vTaskDelay(1000);
            }
            break;

        case ICM20602_FIFO_READ:
            ICM20602_FIFORead();
            break;
    }
}

uint8_t ICM20602_ReadReg(uint8_t reg) {
    uint8_t cmd = reg | READ;
    uint8_t data;
    // Chip selection
    GPIO_Reset(icm20602_cfg.spi.cs_cfg);
    // Transmit one byte with DMA
    SPI_Transmit(&icm20602_cfg.spi, &cmd, 1);
    // Receive one byte with DMA
    SPI_Receive(&icm20602_cfg.spi, &data, 1);
    // Chip deselection
    GPIO_Set(icm20602_cfg.spi.cs_cfg);
    return data;
}

void ICM20602_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = value;
    // Chip selection
    GPIO_Reset(icm20602_cfg.spi.cs_cfg);
    // Transmit two bytes with DMA
    SPI_Transmit(&icm20602_cfg.spi, data, 2);
    // Chip deselection
    GPIO_Set(icm20602_cfg.spi.cs_cfg);
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
            printf("\n0x%02x: 0x%02x (0x%02x not set)\n", 
            (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].setbits);
            rv = 0;
        }

        if((orig_val & reg_cfg[i].clearbits) != 0) {
            printf("\n0x%02x: 0x%02x (0x%02x not cleared)\n", 
            (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    // Set scale and range for processing
    ICM20602_AccelConfigure();
    ICM20602_GyroConfigure();

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

uint16_t ICM20602_FIFOCount() {
    uint8_t cmd = FIFO_COUNTH | READ;
    uint8_t data[2];

    GPIO_Reset(icm20602_cfg.spi.cs_cfg);
    SPI_Transmit(&icm20602_cfg.spi, &cmd, 1);
    SPI_Receive(&icm20602_cfg.spi, data, 2);
    GPIO_Set(icm20602_cfg.spi.cs_cfg);

    return (msblsb16(data[0], data[1]));
}

int ICM20602_FIFORead() {
    uint16_t bytes = ICM20602_FIFOCount();
    uint16_t samples = bytes / sizeof(FIFO_t);
    
    uint8_t cmd = FIFO_COUNTH | READ;

    // Chip selection
    GPIO_Reset(icm20602_cfg.spi.cs_cfg);
    // Transmit cmd
    SPI_Transmit(&icm20602_cfg.spi, &cmd, 1);
    // Receive
    SPI_Receive(&icm20602_cfg.spi, (uint8_t *)&icm_20602_FIFO, bytes);
    // Chip deselection
    GPIO_Set(icm20602_cfg.spi.cs_cfg);

    int16_t accel_x_int16 = msblsb16(icm_20602_FIFO.buf[0].ACCEL_XOUT_H, icm_20602_FIFO.buf[0].ACCEL_XOUT_L);
    float accel_x = accel_x_int16 * icm20602_cfg.dim.accel_scale;

    int16_t accel_y_int16 = msblsb16(icm_20602_FIFO.buf[0].ACCEL_YOUT_H, icm_20602_FIFO.buf[0].ACCEL_YOUT_L);
    float accel_y = accel_y_int16 * icm20602_cfg.dim.accel_scale;

    int16_t accel_z_int16 = msblsb16(icm_20602_FIFO.buf[0].ACCEL_ZOUT_H, icm_20602_FIFO.buf[0].ACCEL_ZOUT_L);
    float accel_z = accel_z_int16 * icm20602_cfg.dim.accel_scale;

    int16_t gyro_x_int16 = msblsb16(icm_20602_FIFO.buf[0].GYRO_XOUT_H, icm_20602_FIFO.buf[0].GYRO_XOUT_L);
    float gyro_x = gyro_x_int16 * icm20602_cfg.dim.gyro_scale;

    int16_t gyro_y_int16 = msblsb16(icm_20602_FIFO.buf[0].GYRO_YOUT_H, icm_20602_FIFO.buf[0].GYRO_YOUT_L);
    float gyro_y = gyro_y_int16 * icm20602_cfg.dim.gyro_scale;

    int16_t gyro_z_int16 = msblsb16(icm_20602_FIFO.buf[0].GYRO_ZOUT_H, icm_20602_FIFO.buf[0].GYRO_ZOUT_L);
    float gyro_z = gyro_z_int16 * icm20602_cfg.dim.gyro_scale;

    int16_t temp_uint16_t = msblsb16(icm_20602_FIFO.buf[0].TEMP_H, icm_20602_FIFO.buf[0].TEMP_L);
    float temp = temp_uint16_t / TEMP_SENS + TEMP_OFFSET;

#ifdef ICM20602_DEBUG
    printf("\naccel_x = %.2f\n", accel_x);
    printf("\naccel_y = %.2f\n", accel_y);
    printf("\naccel_z = %.2f\n", accel_z);
    printf("\ngyro_x  = %.2f\n", gyro_x);
    printf("\ngyro_y  = %.2f\n", gyro_y);
    printf("\ngyro_z  = %.2f\n", gyro_z);
    printf("\ntemp = %.2f\n", temp);
#endif

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

void ICM20602_AccelProcess(){

}

int ICM20602_Probe() {
    uint8_t whoami;
    whoami = ICM20602_ReadReg(WHO_AM_I);
    if(whoami != WHOAMI) {
        printf("unexpected WHO_AM_I reg 0x%02x", whoami);
        return ENODEV;
    }
    return 0;
}

void SPI1_IRQHandler(void) {
    SPI_Handler(&icm20602_cfg.spi);
}

void DMA2_Stream3_IRQHandler(void) {
    DMA_IRQHandler(&dma_spi1_mosi);
}

void DMA2_Stream0_IRQHandler(void) {
    DMA_IRQHandler(&dma_spi1_miso);
}
