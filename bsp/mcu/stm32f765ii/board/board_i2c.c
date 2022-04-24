#include "board.h"
#include "board_cfg.h"

i2c_cfg_t i2c3;
dma_cfg_t i2c3_dma_rx;
dma_cfg_t i2c3_dma_tx;
gpio_cfg_t i2c3_sda = GPIO_I2C3_SDA;
gpio_cfg_t i2c3_scl = GPIO_I2C3_SCL;
const int i2c3_timeout = 20;
const int i2c3_priority = 6;

int I2C3_Init() {
    int rv = 0;

    i2c3.I2C = I2C3;
    i2c3.sda_cfg = &i2c3_sda;
    i2c3.scl_cfg = &i2c3_scl;
    i2c3.timeout = i2c3_timeout;
    i2c3.priority = i2c3_priority;

    i2c3.dma_tx_cfg = &i2c3_dma_tx;
    i2c3.dma_rx_cfg = &i2c3_dma_rx;

    i2c3_dma_tx.DMA_InitStruct.Instance = DMA1_Stream4;
    i2c3_dma_tx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_3;
    i2c3_dma_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    i2c3_dma_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    i2c3_dma_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    i2c3_dma_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    i2c3_dma_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    i2c3_dma_tx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    i2c3_dma_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    i2c3_dma_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    i2c3_dma_tx.priority = i2c3_priority;

    i2c3_dma_rx.DMA_InitStruct.Instance = DMA1_Stream2;
    i2c3_dma_rx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_3;
    i2c3_dma_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    i2c3_dma_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    i2c3_dma_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    i2c3_dma_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    i2c3_dma_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    i2c3_dma_rx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    i2c3_dma_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    i2c3_dma_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    i2c3_dma_rx.priority = i2c3_priority;

    rv |= I2C_Init(&i2c3);

    return rv;
}
