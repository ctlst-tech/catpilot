#include "init.h"
#include "cfg.h"

spi_cfg_t spi1;
dma_cfg_t spi1_dma_rx;
dma_cfg_t spi1_dma_tx;
gpio_cfg_t spi1_mosi = GPIO_SPI1_MOSI;
gpio_cfg_t spi1_miso = GPIO_SPI1_MISO;
gpio_cfg_t spi1_sck  = GPIO_SPI1_SCK;
gpio_cfg_t spi1_cs1  = GPIO_SPI1_CS1;
gpio_cfg_t spi1_cs2  = GPIO_SPI1_CS2;
const int spi1_timeout = 20;
const int spi1_priority = 6;

spi_cfg_t spi2;
dma_cfg_t spi2_dma_rx;
dma_cfg_t spi2_dma_tx;
gpio_cfg_t spi2_mosi = GPIO_SPI2_MOSI;
gpio_cfg_t spi2_miso = GPIO_SPI2_MISO;
gpio_cfg_t spi2_sck  = GPIO_SPI2_SCK;
gpio_cfg_t spi2_cs1  = GPIO_SPI2_CS1;
const int spi2_timeout = 20;
const int spi2_priority = 6;

spi_cfg_t spi4;
dma_cfg_t spi4_dma_rx;
dma_cfg_t spi4_dma_tx;
gpio_cfg_t spi4_mosi = GPIO_SPI4_MOSI;
gpio_cfg_t spi4_miso = GPIO_SPI4_MISO;
gpio_cfg_t spi4_sck  = GPIO_SPI4_SCK;
gpio_cfg_t spi4_cs1  = GPIO_SPI4_CS1;
gpio_cfg_t spi4_cs2  = GPIO_SPI4_CS2;
gpio_cfg_t spi4_cs3  = GPIO_SPI4_CS3;
const int spi4_timeout = 20;
const int spi4_priority = 6;

int SPI1_Init() {
    int rv = 0;

    spi1.SPI = SPI1;
    spi1.mosi_cfg = &spi1_mosi;
    spi1.miso_cfg = &spi1_miso;
    spi1.sck_cfg  = &spi1_sck;
    spi1.timeout  = spi1_timeout;
    spi1.priority = spi1_priority;

    spi1.dma_mosi_cfg = &spi1_dma_tx;
    spi1.dma_miso_cfg = &spi1_dma_rx;

    spi1_dma_tx.DMA_InitStruct.Instance = DMA1_Stream4;
    spi1_dma_tx.DMA_InitStruct.Init.Request = DMA_REQUEST_SPI1_TX;
    spi1_dma_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    spi1_dma_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    spi1_dma_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    spi1_dma_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    spi1_dma_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    spi1_dma_tx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    spi1_dma_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    spi1_dma_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    spi1_dma_tx.priority = spi1_priority;

    spi1_dma_rx.DMA_InitStruct.Instance = DMA1_Stream5;
    spi1_dma_rx.DMA_InitStruct.Init.Request = DMA_REQUEST_SPI1_RX;
    spi1_dma_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    spi1_dma_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    spi1_dma_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    spi1_dma_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    spi1_dma_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    spi1_dma_rx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    spi1_dma_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    spi1_dma_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    spi1_dma_rx.priority = spi1_priority;

    rv |= GPIO_Init(&gpio_spi1_cs1);
    GPIO_Set(&gpio_spi1_cs1);
    rv |= GPIO_Init(&gpio_spi1_cs2);
    GPIO_Set(&gpio_spi1_cs2);
    rv |= EXTI_Init(&exti_spi1_drdy1);
    EXTI_DisableIRQ(&exti_spi1_drdy1);

    rv |= SPI_Init(&spi1);

    return rv;
}

int SPI2_Init() {
    int rv = 0;

    spi2.SPI = SPI2;
    spi2.mosi_cfg = &spi2_mosi;
    spi2.miso_cfg = &spi2_miso;
    spi2.sck_cfg  = &spi2_sck;
    spi2.timeout  = spi2_timeout;
    spi2.priority = spi2_priority;

    spi2.dma_mosi_cfg = &spi2_dma_tx;
    spi2.dma_miso_cfg = &spi2_dma_rx;

    spi2_dma_tx.DMA_InitStruct.Instance = DMA1_Stream6;
    spi2_dma_tx.DMA_InitStruct.Init.Request = DMA_REQUEST_SPI2_TX;
    spi2_dma_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    spi2_dma_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    spi2_dma_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    spi2_dma_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    spi2_dma_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    spi2_dma_tx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    spi2_dma_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    spi2_dma_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    spi2_dma_tx.priority = spi2_priority;

    spi2_dma_rx.DMA_InitStruct.Instance = DMA1_Stream7;
    spi2_dma_rx.DMA_InitStruct.Init.Request = DMA_REQUEST_SPI2_RX;
    spi2_dma_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    spi2_dma_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    spi2_dma_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    spi2_dma_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    spi2_dma_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    spi2_dma_rx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    spi2_dma_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    spi2_dma_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    spi2_dma_rx.priority = spi2_priority;

    rv |= GPIO_Init(&gpio_spi2_cs1);
    GPIO_Set(&gpio_spi2_cs1);

    rv |= SPI_Init(&spi2);

    return rv;
}

int SPI4_Init() {
    int rv = 0;

    spi4.SPI = SPI4;
    spi4.mosi_cfg = &spi4_mosi;
    spi4.miso_cfg = &spi4_miso;
    spi4.sck_cfg  = &spi4_sck;
    spi4.timeout  = spi4_timeout;
    spi4.priority = spi4_priority;

    spi4.dma_mosi_cfg = &spi4_dma_tx;
    spi4.dma_miso_cfg = &spi4_dma_rx;

    spi4_dma_tx.DMA_InitStruct.Instance = DMA2_Stream0;
    spi4_dma_tx.DMA_InitStruct.Init.Request = DMA_REQUEST_SPI4_TX;
    spi4_dma_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    spi4_dma_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    spi4_dma_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    spi4_dma_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    spi4_dma_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    spi4_dma_tx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    spi4_dma_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    spi4_dma_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    spi4_dma_tx.priority = spi4_priority;

    spi4_dma_rx.DMA_InitStruct.Instance = DMA2_Stream1;
    spi4_dma_rx.DMA_InitStruct.Init.Request = DMA_REQUEST_SPI4_RX;
    spi4_dma_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    spi4_dma_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    spi4_dma_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    spi4_dma_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    spi4_dma_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    spi4_dma_rx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    spi4_dma_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    spi4_dma_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    spi4_dma_rx.priority = spi4_priority;

    rv |= GPIO_Init(&gpio_spi4_cs1);
    GPIO_Set(&gpio_spi4_cs1);
    rv |= GPIO_Init(&gpio_spi4_cs2);
    GPIO_Set(&gpio_spi4_cs2);
    rv |= GPIO_Init(&gpio_spi4_cs3);
    GPIO_Set(&gpio_spi4_cs3);

    rv |= SPI_Init(&spi4);

    return rv;
}
