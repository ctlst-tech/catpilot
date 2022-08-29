#include "init.h"

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

/* EXTI Handlers */
void EXTI15_10_IRQHandler(void) {
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

/* UART6 Handlers */
void UART6_IRQHandler(void) {
    USART_Handler(&usart6);
}

void DMA1_Stream0_IRQHandler(void) {
    USART_DMA_TX_Handler(&usart6);
}

void DMA1_Stream6_IRQHandler(void) {
    USART_DMA_RX_Handler(&usart6);
}

/* SDIO Handlers */
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd) {
    SDIO_RX_Complete(&sdmmc1);
}

void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd) {
    SDIO_TX_Complete(&sdmmc1);
}

void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd) {
    while(1);
}

void SDMMC1_IRQHandler(void) {
    SDIO_IT_Handler(&sdmmc1);
}
