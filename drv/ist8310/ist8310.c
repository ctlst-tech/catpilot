#include "ist8310.h"
#include "ist8310_reg.h"

static char *device = "ICM20602";

gpio_cfg_t ist8310_sda = GPIO_I2C3_SDA;
gpio_cfg_t ist8310_scl = GPIO_I2C3_SCL;

dma_cfg_t dma_i2c3_tx;
dma_cfg_t dma_i2c3_rx;

ist8310_cfg_t ist8310_cfg;

enum ist8310_state_t {
    IST8310_RESET,
    IST8310_RESET_WAIT,
    IST8310_CONF,
    IST8310_MEAS,
    IST8310_READ
};

enum ist8310_state_t ist8310_state;

int IST8310_Init() {
    int rv = 0;

    ist8310_cfg.i2c.I2C = I2C3;
    ist8310_cfg.i2c.sda_cfg = &ist8310_sda;
    ist8310_cfg.i2c.scl_cfg = &ist8310_scl;
    ist8310_cfg.i2c.timeout = 20;
    ist8310_cfg.i2c.priority = 6;

    ist8310_cfg.i2c.dma_tx_cfg = &dma_i2c3_tx;
    ist8310_cfg.i2c.dma_rx_cfg = &dma_i2c3_rx;

    dma_i2c3_tx.DMA_InitStruct.Instance = DMA1_Stream0;
    dma_i2c3_tx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_8;
    dma_i2c3_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma_i2c3_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_i2c3_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_i2c3_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_i2c3_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_i2c3_tx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    dma_i2c3_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    dma_i2c3_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_i2c3_tx.priority = ist8310_cfg.i2c.priority;

    dma_i2c3_rx.DMA_InitStruct.Instance = DMA1_Stream2;
    dma_i2c3_rx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_3;
    dma_i2c3_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_i2c3_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_i2c3_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_i2c3_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_i2c3_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_i2c3_rx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    dma_i2c3_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    dma_i2c3_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_i2c3_rx.priority = ist8310_cfg.i2c.priority;

    rv |= I2C_Init(&ist8310_cfg.i2c);

    ist8310_state = IST8310_RESET;

    return rv;
}

void IST8310_Run() {
    switch(ist8310_state) {

    case IST8310_RESET:
        IST8310_WriteReg(CNTL2, SRST);
        ist8310_state = IST8310_RESET_WAIT;
        vTaskDelay(30);
        break;

    case IST8310_RESET_WAIT:
        if ((IST8310_ReadReg(WHO_AM_I) == DEVICE_ID)
            && ((IST8310_ReadReg(CNTL2) & SRST) == 0)) {
                ist8310_state = IST8310_CONF;
                vTaskDelay(10);
            } else {
                printf("\n%s:Wrong default registers values after reset\n", device);
                vTaskDelay(1000);
            }
        break;

    case IST8310_CONF:
        if(IST8310_Configure()) {
            ist8310_state = IST8310_MEAS;
        } else {
            printf("\n%sWrong configuration, reset\n", device);
            ist8310_state = IST8310_RESET;
            vTaskDelay(1000);
        }
        break;

    case IST8310_MEAS:

        break;
    }
}

uint8_t IST8310_ReadReg(uint8_t reg) {
    uint8_t buf[2];

    buf[0] = (ADDRESS << 1) | READ;
    buf[1] = reg;

    I2C_Transmit(&ist8310_cfg.i2c, buf[0], &buf[1], 1);
    I2C_Receive(&ist8310_cfg.i2c, buf[0], &buf[1], 1);

    return buf[1];
}

void IST8310_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t buf[3];

    buf[0] = (ADDRESS << 1) | WRITE;
    buf[1] = reg;
    buf[2] = value;

    I2C_Transmit(&ist8310_cfg.i2c, buf[0], &buf[1], 2);
}

void IST8310_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits) {
    uint8_t orig_val = IST8310_ReadReg(reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        IST8310_WriteReg(reg, val);
    }
}

int IST8310_Configure() {
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
            printf("%s\n0x%02x: 0x%02x (0x%02x not set)\n", device,
            (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].setbits);
            rv = 0;
        }

        if((orig_val & reg_cfg[i].clearbits) != 0) {
            printf("%s\n0x%02x: 0x%02x (0x%02x not cleared)\n", device,
            (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    return rv;
}

int IST8310_Probe() {
    uint8_t whoami;
    whoami = IST8310_ReadReg(WHO_AM_I);
    if(whoami != DEVICE_ID) {
        printf("unexpected WHO_AM_I reg 0x%02x", whoami);
        return ENODEV;
    }
    return 0;
}

void I2C3_EV_IRQHandler(void) {
    I2C_EV_Handler(&ist8310_cfg.i2c);
}

void I2C3_ER_IRQHandler(void) {
    I2C_ER_Handler(&ist8310_cfg.i2c);
}

void DMA1_Stream0_IRQHandler(void) {
    I2C_DMA_TX_Handler(&ist8310_cfg.i2c);
}

void DMA1_Stream2_IRQHandler(void) {
    I2C_DMA_RX_Handler(&ist8310_cfg.i2c);
}
