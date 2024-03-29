#include "core.h"
#include "hal.h"
#include "board.h"

i2c_t i2c1 = {
    .init = {
        .Instance = I2C1,
        .Init = {
            .AddressingMode = I2C_ADDRESSINGMODE_7BIT,
            .DualAddressMode = I2C_DUALADDRESS_DISABLE,
            .GeneralCallMode = I2C_GENERALCALL_DISABLE,
            .NoStretchMode = I2C_NOSTRETCH_DISABLE,
            .Timing = 0x00602173,
            .OwnAddress1 = 0,
            .OwnAddress2 = 0,
            .OwnAddress2Masks = I2C_OA2_NOMASK,
        },
    },
    .dma_tx = {
        .init = {
            .Instance = DMA2_Stream0,
            .Init.Request = DMA_REQUEST_I2C1_TX,
            .Init.Direction = DMA_MEMORY_TO_PERIPH,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_VERY_HIGH,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE,
        },
        .irq_priority = 9
    },
    .dma_rx = {
        .init = {
            .Instance = DMA2_Stream1,
            .Init.Request = DMA_REQUEST_I2C1_RX,
            .Init.Direction = DMA_PERIPH_TO_MEMORY,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_VERY_HIGH,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE,
        },
        .irq_priority = 9
    },
    .scl = &gpio_i2c1_scl,
    .sda = &gpio_i2c1_sda,
    .timeout = 10,
    .irq_priority = 9,
    .p = {0}
};

i2c_t i2c2 = {
    .init = {
        .Instance = I2C2,
        .Init = {
            .AddressingMode = I2C_ADDRESSINGMODE_7BIT,
            .DualAddressMode = I2C_DUALADDRESS_DISABLE,
            .GeneralCallMode = I2C_GENERALCALL_DISABLE,
            .NoStretchMode = I2C_NOSTRETCH_DISABLE,
            .Timing = 0x00602173
        },
    },
    .dma_tx = {
        .init = {
            .Instance = DMA2_Stream2,
            .Init.Request = DMA_REQUEST_I2C2_TX,
            .Init.Direction = DMA_MEMORY_TO_PERIPH,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_VERY_HIGH,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE,
        },
        .irq_priority = 9
    },
    .dma_rx = {
        .init = {
            .Instance = DMA2_Stream3,
            .Init.Request = DMA_REQUEST_I2C2_RX,
            .Init.Direction = DMA_PERIPH_TO_MEMORY,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_VERY_HIGH,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE,
        },
        .irq_priority = 9
    },
    .scl = &gpio_i2c2_scl,
    .sda = &gpio_i2c2_sda,
    .timeout = 10,
    .irq_priority = 9,
    .p = {0}
};
