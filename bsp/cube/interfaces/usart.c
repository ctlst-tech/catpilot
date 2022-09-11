#include "init.h"
#include "cfg.h"

usart_cfg_t usart2;
dma_cfg_t usart2_dma_tx;
dma_cfg_t usart2_dma_rx;
gpio_cfg_t usart2_tx = GPIO_USART2_TX;
gpio_cfg_t usart2_rx = GPIO_USART2_RX;
const int usart2_bitrate = 115200;
const int usart2_timeout = portMAX_DELAY;
const int usart2_priority = 15;
const int usart2_task_priority = 1;

usart_cfg_t usart6;
dma_cfg_t usart6_dma_tx;
dma_cfg_t usart6_dma_rx;
gpio_cfg_t usart6_tx = GPIO_USART6_TX;
gpio_cfg_t usart6_rx = GPIO_USART6_RX;
const int usart6_bitrate = 1500000;
const int usart6_timeout = 20;
const int usart6_priority = 15;

int USART2_Init() {
    int rv = 0;

    usart2.USART = USART2;
    usart2.gpio_tx_cfg = &usart2_tx;
    usart2.gpio_rx_cfg = &usart2_rx;
    usart2.dma_tx_cfg = &usart2_dma_tx;
    usart2.dma_rx_cfg = &usart2_dma_rx;
    usart2.speed = usart2_bitrate;
    usart2.timeout = usart2_timeout;
    usart2.priority = usart2_priority;
    usart2.buf_size = 1024;
    usart2.mode = USART_IDLE;
    usart2.task_priority = usart2_task_priority;

    usart2_dma_tx.DMA_InitStruct.Instance = DMA1_Stream0;
    usart2_dma_tx.DMA_InitStruct.Init.Request = DMA_REQUEST_USART2_TX;
    usart2_dma_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    usart2_dma_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    usart2_dma_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    usart2_dma_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    usart2_dma_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    usart2_dma_tx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    usart2_dma_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    usart2_dma_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    usart2_dma_tx.priority = usart2_priority;

    usart2_dma_rx.DMA_InitStruct.Instance = DMA1_Stream1;
    usart2_dma_rx.DMA_InitStruct.Init.Request = DMA_REQUEST_USART2_RX;
    usart2_dma_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    usart2_dma_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    usart2_dma_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    usart2_dma_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    usart2_dma_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    usart2_dma_rx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    usart2_dma_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    usart2_dma_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    usart2_dma_rx.priority = usart2_priority;

    rv = USART_Init(&usart2);

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

    usart6_dma_tx.DMA_InitStruct.Instance = DMA1_Stream2;
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

    usart6_dma_rx.DMA_InitStruct.Instance = DMA1_Stream3;
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
