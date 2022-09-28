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

void DMA2_Stream6_IRQHandler(void) {
    SDIO_DMA_Handler(&sdmmc1);
}

/** EXTI Handlers */
void EXTI9_5_IRQHandler(void) {
    uint32_t line = EXTI->PR;
    if(line & GPIO_PIN_5) {
        ICM20602_DataReadyHandler();
    }
}

void EXTI4_IRQHandler(void) {
    uint32_t line = EXTI->PR;
    if(line & GPIO_PIN_4) {
        ICM20689_DataReadyHandler();
    }
}
