#include "board.h"
#include "board_cfg.h"

spi_cfg_t spi1;
dma_cfg_t spi1_dma_rx;
dma_cfg_t spi1_dma_tx;
gpio_cfg_t spi1_mosi = GPIO_SPI1_MOSI;
gpio_cfg_t spi1_miso = GPIO_SPI1_MISO;
gpio_cfg_t spi1_sck  = GPIO_SPI1_SCK;
gpio_cfg_t spi1_cs1  = GPIO_SPI1_CS1;
gpio_cfg_t spi1_cs2  = GPIO_SPI1_CS2;
gpio_cfg_t spi1_cs3  = GPIO_SPI1_CS3;
gpio_cfg_t spi1_cs4  = GPIO_SPI1_CS4;
const int spi1_timeout = 20;
const int spi1_priority = 6;

int SPI1_Init() {
    int rv = 0;

    GPIO_Init(&spi1_cs1);
    GPIO_Init(&spi1_cs2);
    GPIO_Init(&spi1_cs3);
    GPIO_Init(&spi1_cs4);
    GPIO_Set(&spi1_cs1);
    GPIO_Set(&spi1_cs2);
    GPIO_Set(&spi1_cs3);
    GPIO_Set(&spi1_cs4);

    spi1.SPI = SPI1;
    spi1.mosi_cfg = &spi1_mosi;
    spi1.miso_cfg = &spi1_miso;
    spi1.sck_cfg  = &spi1_sck;
    spi1.timeout  = spi1_timeout;
    spi1.priority = spi1_priority;

    spi1.dma_mosi_cfg = &spi1_dma_tx;
    spi1.dma_miso_cfg = &spi1_dma_rx;

    spi1_dma_tx.DMA_InitStruct.Instance = DMA2_Stream3;
    spi1_dma_tx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_3;
    spi1_dma_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    spi1_dma_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    spi1_dma_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    spi1_dma_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    spi1_dma_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    spi1_dma_tx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    spi1_dma_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    spi1_dma_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    spi1_dma_tx.priority = spi1_priority;

    spi1_dma_rx.DMA_InitStruct.Instance = DMA2_Stream0;
    spi1_dma_rx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_3;
    spi1_dma_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    spi1_dma_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    spi1_dma_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    spi1_dma_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    spi1_dma_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    spi1_dma_rx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    spi1_dma_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    spi1_dma_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    spi1_dma_rx.priority = spi1_priority;

    rv = SPI_Init(&spi1);

    return rv;
}

void SPI1_ChipSelect(int num) {
    switch(num) {

    case 1:
        GPIO_Reset(&spi1_cs1);
        GPIO_Set(&spi1_cs2);
        GPIO_Set(&spi1_cs3);
        GPIO_Set(&spi1_cs4);
        break;

    case 2:
        GPIO_Set(&spi1_cs1);
        GPIO_Reset(&spi1_cs2);
        GPIO_Set(&spi1_cs3);
        GPIO_Set(&spi1_cs4);
        break;

    case 3:
        GPIO_Set(&spi1_cs1);
        GPIO_Set(&spi1_cs2);
        GPIO_Reset(&spi1_cs3);
        GPIO_Set(&spi1_cs4);
        break;

    case 4:
        GPIO_Set(&spi1_cs1);
        GPIO_Set(&spi1_cs2);
        GPIO_Set(&spi1_cs3);
        GPIO_Reset(&spi1_cs4);
        break;

    default:
        break;
    }
}

void SPI1_ChipDeselect() {
    GPIO_Set(&spi1_cs1);
    GPIO_Set(&spi1_cs2);
    GPIO_Set(&spi1_cs3);
    GPIO_Set(&spi1_cs4);
}
