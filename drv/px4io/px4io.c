#include "px4io.h"
#include "px4io_conf.h"
#include "px4io_protocol.h"

static char *device = "PX4IO";

static gpio_cfg_t px4io_tx = GPIO_USART8_TX;
static gpio_cfg_t px4io_rx = GPIO_USART8_RX;

static dma_cfg_t dma_px4io_tx;
static dma_cfg_t dma_px4io_rx;

static px4io_cfg_t px4io_cfg;

int PXIO_Init() {
    int rv = 0;

    GPIO_Init(&px4io_tx);
    GPIO_Init(&px4io_rx);

    px4io_cfg.usart.USART = UART8;
    px4io_cfg.usart.gpio_tx_cfg = &px4io_tx;
    px4io_cfg.usart.gpio_rx_cfg = &px4io_rx;
    px4io_cfg.usart.dma_tx_cfg = &dma_px4io_tx;
    px4io_cfg.usart.dma_rx_cfg = &dma_px4io_rx;
    px4io_cfg.usart.speed = PX4IO_SERIAL_BITRATE;
    px4io_cfg.usart.timeout = PX4IO_SERIAL_TIMEOUT;
    px4io_cfg.usart.priority = PX4IO_SERIAL_PRIORITY;

    dma_px4io_tx.DMA_InitStruct.Instance = DMA1_Stream0;
    dma_px4io_tx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_5;
    dma_px4io_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma_px4io_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_px4io_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_px4io_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_px4io_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_px4io_tx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    dma_px4io_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    dma_px4io_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_px4io_tx.priority = PX4IO_SERIAL_PRIORITY;

    dma_px4io_rx.DMA_InitStruct.Instance = DMA1_Stream6;
    dma_px4io_rx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_5;
    dma_px4io_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_px4io_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_px4io_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_px4io_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_px4io_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_px4io_rx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    dma_px4io_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    dma_px4io_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_px4io_rx.priority = PX4IO_SERIAL_PRIORITY;

    rv = USART_Init(&px4io_cfg.usart);

    return rv;
}

int PXIO_Write(uint8_t address, uint8_t *data, uint8_t length) {
    int rv = 0;

    return rv;
}

int PXIO_Read(uint8_t address, uint8_t *data, uint8_t length) {
    int rv = 0;

    return rv;
}

void UART8_IRQHandler(void) {
    USART_Handler(&px4io_cfg.usart);
}

void DMA1_Stream0_IRQHandler(void) {
    DMA_IRQHandler(&dma_px4io_tx);
}

void DMA1_Stream6_IRQHandler(void) {
    DMA_IRQHandler(&dma_px4io_rx);
}