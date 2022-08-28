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

usart_cfg_t usart6;
dma_cfg_t usart6_dma_tx;
dma_cfg_t usart6_dma_rx;
gpio_cfg_t usart6_tx = GPIO_USART6_TX;
gpio_cfg_t usart6_rx = GPIO_USART6_RX;
const int usart6_bitrate = 1500000;
const int usart6_timeout = 20;
const int usart6_priority = 15;

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
    usart7_dma_tx.DMA_InitStruct.Init.Request = DMA_REQUEST_UART7_TX;
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
    usart7_dma_rx.DMA_InitStruct.Init.Request = DMA_REQUEST_UART7_RX;
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

int USART6_Init() {
    int rv = 0;

    usart6.USART = USART6;
    usart6.gpio_tx_cfg = &usart6_tx;
    usart6.gpio_rx_cfg = &usart6_rx;
    usart6.dma_tx_cfg = &usart6_dma_tx;
    usart6.dma_rx_cfg = &usart6_dma_rx;
    usart6.speed = usart6_bitrate;
    usart6.timeout = usart6_timeout;
    usart6.priority = usart6_priority;
    usart6.mode = USART_IDLE;

    usart6_dma_tx.DMA_InitStruct.Instance = DMA1_Stream0;
    usart6_dma_tx.DMA_InitStruct.Init.Request = DMA_REQUEST_USART6_TX;
    usart6_dma_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    usart6_dma_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    usart6_dma_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    usart6_dma_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    usart6_dma_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    usart6_dma_tx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    usart6_dma_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    usart6_dma_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    usart6_dma_tx.priority = usart6_priority;

    usart6_dma_rx.DMA_InitStruct.Instance = DMA1_Stream6;
    usart6_dma_rx.DMA_InitStruct.Init.Request = DMA_REQUEST_USART6_RX;
    usart6_dma_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    usart6_dma_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    usart6_dma_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    usart6_dma_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    usart6_dma_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    usart6_dma_rx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    usart6_dma_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    usart6_dma_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    usart6_dma_rx.priority = usart6_priority;

    rv = USART_Init(&usart6);

    return rv;
}
