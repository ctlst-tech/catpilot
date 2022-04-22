#include "board.h"

spi_cfg_t spi1;
i2c_cfg_t i2c3;
usart_cfg_t usart7;
usart_cfg_t usart8;

int USART7_Init();
int USART8_Init();
int SPI1_Init();
int I2C3_Init();

int Board_Init() {
    int rv = 0;
    if(SART7_Init()) {
        return rv;
    }
    if(USART8_Init()) {
        return rv;
    }
    if(SPI1_Init()) {
        return rv;
    }
    if(I2C3_Init()) {
        return rv;
    }
    return rv;
}

int USART7_Init() {
    int rv = 0;

    usart7.USART = UART7;
    usart7.gpio_tx_cfg = &gpio_cli_tx;
    usart7.gpio_rx_cfg = &gpio_cli_rx;
    usart7.dma_tx_cfg = &dma_cli_tx;
    usart7.dma_rx_cfg = &dma_cli_rx;
    usart7.speed = CLI_BITRATE;
    usart7.timeout = CLI_TIMEOUT;
    usart7.priority = CLI_IRQ_PRIORITY;;
    usart7.mode = USART_TIMEOUT;

    usart7.DMA_InitStruct.Instance = DMA1_Stream1;
    usart7.DMA_InitStruct.Init.Channel = DMA_CHANNEL_5;
    usart7.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    usart7.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    usart7.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    usart7.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    usart7.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    usart7.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    usart7.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    usart7.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    usart7.priority = CLI_IRQ_PRIORITY;

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
}

/* SPI1 Handlers */
void SPI1_IRQHandler(void) {
    SPI_IT_Handler(&spi1);
}

void DMA2_Stream3_IRQHandler(void) {
    SPI_DMA_MOSI_Handler(&spi1);
}

void DMA2_Stream0_IRQHandler(void) {
    SPI_DMA_MISO_Handler(&spi1);
}

/* I2C3 Handlers */
void I2C3_EV_IRQHandler(void) {
    I2C_EV_Handler(&i2c3);
}

void I2C3_ER_IRQHandler(void) {
    I2C_ER_Handler(&i2c3);
}

void DMA1_Stream4_IRQHandler(void) {
    I2C_DMA_TX_Handler(&i2c3);
}

void DMA1_Stream2_IRQHandler(void) {
    I2C_DMA_RX_Handler(&i2c3);
}

/* UART7 Handlers */
void UART7_IRQHandler(void) {
    USART_Handler(&usart7);
}

void DMA1_Stream1_IRQHandler(void) {
    USART_DMA_TX_Handler(&usart7);
}

void DMA1_Stream3_IRQHandler(void) {
    USART_DMA_RX_Handler(&usart7);
}

/* UART8 Handlers */
void UART8_IRQHandler(void) {
    USART_Handler(&usart8);
}

void DMA1_Stream0_IRQHandler(void) {
    USART_DMA_TX_Handler(&usart8);
}

void DMA1_Stream6_IRQHandler(void) {
    USART_DMA_RX_Handler(&usart8);
}

/* SDIO Handlers */
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd) {
    SDIO_RX_Complete(&sdio);
}

void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd) {
    SDIO_TX_Complete(&sdio);
}

void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd) {
    while(1);
}

void SDMMC1_IRQHandler(void) {
    SDIO_IT_Handler(&sdio);
}

void DMA2_Stream6_IRQHandler(void) {
    SDIO_DMA_Handler(&sdio);
}
