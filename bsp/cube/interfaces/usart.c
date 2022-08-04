#include "init.h"
#include "cfg.h"

usart_cfg_t usart7;
dma_cfg_t usart7_dma_tx;
dma_cfg_t usart7_dma_rx;
gpio_cfg_t usart7_tx = GPIO_USART7_TX;
gpio_cfg_t usart7_rx = GPIO_USART7_RX;
const int usart7_bitrate = 115200;
const int usart7_timeout = portMAX_DELAY;
const int usart7_priority = 15;
const int usart7_task_priority = 1;

usart_cfg_t usart8;
dma_cfg_t usart8_dma_tx;
dma_cfg_t usart8_dma_rx;
gpio_cfg_t usart8_tx = GPIO_USART8_TX;
gpio_cfg_t usart8_rx = GPIO_USART8_RX;
const int usart8_bitrate = 1500000;
const int usart8_timeout = 20;
const int usart8_priority = 15;

int USART7_Init() {
    int rv = 0;

    usart7.USART = UART7;
    usart7.gpio_tx_cfg = &usart7_tx;
    usart7.gpio_rx_cfg = &usart7_rx;
    usart7.dma_tx_cfg = &usart7_dma_tx;
    usart7.dma_rx_cfg = &usart7_dma_rx;
    usart7.speed = usart7_bitrate;
    usart7.timeout = usart7_timeout;
    usart7.priority = usart7_priority;
    usart7.buf_size = 1024;
    usart7.mode = USART_IDLE;
    usart7.task_priority = usart7_task_priority;

    usart7_dma_tx.DMA_InitStruct.Instance = DMA1_Stream1;
    usart7_dma_tx.DMA_InitStruct.Init.Request = DMA_REQUEST_USART6_RX; //5;
    usart7_dma_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    usart7_dma_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    usart7_dma_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    usart7_dma_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    usart7_dma_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    usart7_dma_tx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    usart7_dma_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    usart7_dma_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    usart7_dma_tx.priority = usart7_priority;

    usart7_dma_rx.DMA_InitStruct.Instance = DMA1_Stream3;
    usart7_dma_rx.DMA_InitStruct.Init.Request = DMA_REQUEST_USART6_RX; //5;
    usart7_dma_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    usart7_dma_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    usart7_dma_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    usart7_dma_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    usart7_dma_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    usart7_dma_rx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    usart7_dma_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    usart7_dma_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    usart7_dma_rx.priority = usart7_priority;

    rv = USART_Init(&usart7);

    return rv;
}

int USART8_Init() {
    int rv = 0;

    usart8.USART = UART8;
    usart8.gpio_tx_cfg = &usart8_tx;
    usart8.gpio_rx_cfg = &usart8_rx;
    usart8.dma_tx_cfg = &usart8_dma_tx;
    usart8.dma_rx_cfg = &usart8_dma_rx;
    usart8.speed = usart8_bitrate;
    usart8.timeout = usart8_timeout;
    usart8.priority = usart8_priority;
    usart8.mode = USART_IDLE;

    usart8_dma_tx.DMA_InitStruct.Instance = DMA1_Stream0;
    usart8_dma_tx.DMA_InitStruct.Init.Request = DMA_REQUEST_USART6_RX; //5;
    usart8_dma_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    usart8_dma_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    usart8_dma_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    usart8_dma_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    usart8_dma_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    usart8_dma_tx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    usart8_dma_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    usart8_dma_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    usart8_dma_tx.priority = usart8_priority;

    usart8_dma_rx.DMA_InitStruct.Instance = DMA1_Stream6;
    usart8_dma_rx.DMA_InitStruct.Init.Request = DMA_REQUEST_USART6_RX; //5;
    usart8_dma_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    usart8_dma_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    usart8_dma_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    usart8_dma_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    usart8_dma_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    usart8_dma_rx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    usart8_dma_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    usart8_dma_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    usart8_dma_rx.priority = usart8_priority;

    rv = USART_Init(&usart8);

    return rv;
}
