#include "core.h"
#include "hal.h"
#include "board.h"

// SPI1 
spi_t spi1 = {
    .init = {
        .Instance = SPI1,
        .Init = {
            .Mode = SPI_MODE_MASTER,
            .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16,
            .CLKPhase = SPI_PHASE_1EDGE,
            .CLKPolarity = SPI_POLARITY_LOW,
            .DataSize = SPI_DATASIZE_8BIT,
            .Direction = SPI_DIRECTION_2LINES,
            .FirstBit = SPI_FIRSTBIT_MSB,
            .MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE,
            .MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE,
            .NSS = SPI_NSS_SOFT,
            .TIMode = SPI_TIMODE_DISABLE,
            .NSSPMode = SPI_NSS_PULSE_DISABLE,
            .CRCCalculation = SPI_CRCCALCULATION_DISABLE
        }
    },
    .dma_tx = {
        .init = {
            .Instance = DMA1_Stream0,
            .Init.Request = DMA_REQUEST_SPI1_TX,
            .Init.Direction = DMA_MEMORY_TO_PERIPH,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_VERY_HIGH,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE
        },
        .irq_priority = 6
    },
    .dma_rx = {
        .init = {
            .Instance = DMA1_Stream1,
            .Init.Request = DMA_REQUEST_SPI1_RX,
            .Init.Direction = DMA_PERIPH_TO_MEMORY,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_VERY_HIGH,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE,
        },
        .irq_priority = 6
    },
    .mosi = &gpio_spi1_mosi,
    .miso = &gpio_spi1_miso,
    .sck = &gpio_spi1_sck,
    .timeout = 10,
    .irq_priority = 6,
    .p = {0}
};

// SPI4 
spi_t spi4 = {
    .init = {
        .Instance = SPI4,
        .Init = {
            .Mode = SPI_MODE_MASTER,
            .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16,
            .CLKPhase = SPI_PHASE_1EDGE,
            .CLKPolarity = SPI_POLARITY_LOW,
            .DataSize = SPI_DATASIZE_8BIT,
            .Direction = SPI_DIRECTION_2LINES,
            .FirstBit = SPI_FIRSTBIT_MSB,
            .MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE,
            .MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE,
            .NSS = SPI_NSS_SOFT,
            .TIMode = SPI_TIMODE_DISABLE,
            .NSSPMode = SPI_NSS_PULSE_DISABLE,
            .CRCCalculation = SPI_CRCCALCULATION_DISABLE
        }
    },
    .dma_tx = {
        .init = {
            .Instance = DMA1_Stream2,
            .Init.Request = DMA_REQUEST_SPI4_TX,
            .Init.Direction = DMA_MEMORY_TO_PERIPH,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_VERY_HIGH,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE
        },
        .irq_priority = 6
    },
    .dma_rx = {
        .init = {
            .Instance = DMA1_Stream3,
            .Init.Request = DMA_REQUEST_SPI4_RX,
            .Init.Direction = DMA_PERIPH_TO_MEMORY,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_VERY_HIGH,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE,
        },
        .irq_priority = 6
    },
    .mosi = &gpio_spi4_mosi,
    .miso = &gpio_spi4_miso,
    .sck = &gpio_spi4_sck,
    .timeout = 10,
    .irq_priority = 6,
    .p = {0}
};
