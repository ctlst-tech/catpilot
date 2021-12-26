#include "cli.h"

gpio_cfg_t gpio_cli_tx = GPIO_USART7_TX;
gpio_cfg_t gpio_cli_rx = GPIO_USART7_RX;
dma_cfg_t dma_cli_tx;
dma_cfg_t dma_cli_rx;

usart_cfg_t cli_cfg;

int CLI_Init() {
    int rv = 0;

    cli_cfg.USART = UART7;
    cli_cfg.gpio_tx_cfg = &gpio_cli_tx;
    cli_cfg.gpio_rx_cfg = &gpio_cli_rx;
    cli_cfg.dma_tx_cfg = &dma_cli_tx;
    cli_cfg.dma_rx_cfg = &dma_cli_rx;
    cli_cfg.speed = 115200;
    cli_cfg.timeout = 20;
    cli_cfg.priority = 15;

    dma_cli_tx.DMA_InitStruct.Instance = DMA1_Stream1;
    dma_cli_tx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_5;
    dma_cli_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma_cli_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_cli_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_cli_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_cli_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_cli_tx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    dma_cli_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    dma_cli_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_cli_tx.priority = 15;

    dma_cli_rx.DMA_InitStruct.Instance = DMA1_Stream3;
    dma_cli_rx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_5;
    dma_cli_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_cli_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_cli_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_cli_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_cli_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_cli_rx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    dma_cli_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW; 
    dma_cli_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_cli_rx.priority = 15;

    USART_Init(&cli_cfg);

    return rv;
}

void retarget_put_char(uint8_t c) {
    USART_Transmit(&cli_cfg, &c, 1);
}

void UART7_IRQHandler(void) {
    USART_Handler(&cli_cfg);
}

void DMA1_Stream1_IRQHandler(void) {
    DMA_IRQHandler(&dma_cli_tx);
}

void DMA1_Stream3_IRQHandler(void) {
    DMA_IRQHandler(&dma_cli_rx);
}
