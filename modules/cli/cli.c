#include "cli.h"
#include "cli_conf.h"

static char *device = "CLI";

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
    cli_cfg.speed = CLI_BITRATE;
    cli_cfg.timeout = CLI_TIMEOUT;
    cli_cfg.priority = CLI_IRQ_PRIORITY;;
    cli_cfg.mode = USART_TIMEOUT;

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
    dma_cli_tx.priority = CLI_IRQ_PRIORITY;

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
    dma_cli_rx.priority = CLI_IRQ_PRIORITY;

    rv = USART_Init(&cli_cfg);

    return rv;
}

void CLI_Task() {
    int rv = 0;
    uint8_t data;

    rv = CLI_Init();

    if(rv) {
        // Delete task
    }

    while(1) {
        vTaskDelay(1000);
        // Nothing to do
    }
}

void CLI_Start() {
    xTaskCreate(CLI_Task, "CLI", 512, NULL, CLI_TASK_PRIORITY, NULL);
}

void retarget_put_char(uint8_t c) {
    USART_Transmit(&cli_cfg, &c, 1);
}

int cli_put(char c, struct __file * file) {
    (void)file;
    USART_Transmit(&cli_cfg, (uint8_t *)&c, 1);
    file->len--;
    if(c == '\n') USART_Transmit(&cli_cfg, (uint8_t *)"\r", 1);
    return 0;
}

// int cli_get(char c, struct __file * file) {
//     (void)file;
//     USART_Receive(&cli_cfg, &c, 1);
// }

void UART7_IRQHandler(void) {
    USART_Handler(&cli_cfg);
}

void DMA1_Stream1_IRQHandler(void) {
    DMA_IRQHandler(&dma_cli_tx);
}

void DMA1_Stream3_IRQHandler(void) {
    DMA_IRQHandler(&dma_cli_rx);
}
