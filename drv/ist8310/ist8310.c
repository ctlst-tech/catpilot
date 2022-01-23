#include "ist8310.h"
#include "ist8310_reg.h"

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

void I2C3_EV_IRQHandler(void) {
    I2C_EV_Handler(&ist8310_cfg.i2c);
}

void I2C3_ER_IRQHandler(void) {
    I2C_ER_Handler(&ist8310_cfg.i2c);
}

void DMA1_Stream0_IRQHandler(void) {
    DMA_IRQHandler(&dma_i2c3_tx);
}

void DMA1_Stream2_IRQHandler(void) {
    DMA_IRQHandler(&dma_i2c3_rx);
}
