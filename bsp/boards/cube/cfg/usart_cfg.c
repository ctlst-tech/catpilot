#include "core.h"
#include "hal.h"
#include "board.h"

// TELEM1
usart_t usart2 = {
    .init = {
        .Instance = USART2,
        .Init = {
            .BaudRate = 57600,
            .ClockPrescaler = 1,
            .Mode = UART_MODE_TX_RX,
            .OverSampling = UART_OVERSAMPLING_16,
            .Parity = UART_PARITY_NONE,
            .StopBits = UART_STOPBITS_1,
            .WordLength = UART_WORDLENGTH_8B
        }
    },
    .dma_tx = {
        .init = {
            .Instance = DMA1_Stream1,
            .Init = {
                .Request = DMA_REQUEST_USART2_TX,
                .Direction = DMA_MEMORY_TO_PERIPH,
                .PeriphInc = DMA_PINC_DISABLE,
                .MemInc = DMA_MINC_ENABLE,
                .PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
                .MemDataAlignment = DMA_MDATAALIGN_BYTE,
                .Mode = DMA_NORMAL,
                .Priority = DMA_PRIORITY_LOW,
                .FIFOMode = DMA_FIFOMODE_DISABLE
            }
        }
    },
    .dma_rx = {
        .init = {
            .Instance = DMA1_Stream2,
            .Init = {
                .Request = DMA_REQUEST_USART2_RX,
                .Direction = DMA_PERIPH_TO_MEMORY,
                .PeriphInc = DMA_PINC_DISABLE,
                .MemInc = DMA_MINC_ENABLE,
                .PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
                .MemDataAlignment = DMA_MDATAALIGN_BYTE,
                .Mode = DMA_NORMAL,
                .Priority = DMA_PRIORITY_LOW,
                .FIFOMode = DMA_FIFOMODE_DISABLE
            }
            
        }
    },
    .mode = USART_IDLE,
    .gpio_tx = &gpio_usart2_tx,
    .gpio_rx = &gpio_usart2_rx,
    .timeout = 50,
    .buf_size = 1024,
    .irq_priority = 7,
    .task_priority = 2,
    .p = {0}
};

// TELEM2
usart_t usart3 = {
    .init = {
        .Instance = USART3,
        .Init.BaudRate = 115200,
        .Init.ClockPrescaler = 1,
        .Init.Mode = UART_MODE_TX_RX,
        .Init.OverSampling = UART_OVERSAMPLING_16,
        .Init.Parity = UART_PARITY_NONE,
        .Init.StopBits = UART_STOPBITS_1,
        .Init.WordLength = UART_WORDLENGTH_8B
    },
    .mode = USART_IDLE,
    .gpio_tx = &gpio_usart3_tx,
    .gpio_rx = &gpio_usart3_rx,
    .timeout = 50,
    .buf_size = 1024,
    .irq_priority = 7,
    .task_priority = 2,
    .p = {0}
};

// IO
usart_t usart6 = {
    .init = {
        .Instance = USART6,
        .Init.BaudRate = 1500000,
        .Init.ClockPrescaler = 1,
        .Init.Mode = UART_MODE_TX_RX,
        .Init.OverSampling = UART_OVERSAMPLING_16,
        .Init.Parity = UART_PARITY_NONE,
        .Init.StopBits = UART_STOPBITS_1,
        .Init.WordLength = UART_WORDLENGTH_8B
    },
    .mode = USART_IDLE,
    .gpio_tx = &gpio_usart6_tx,
    .gpio_rx = &gpio_usart6_rx,
    .timeout = 20,
    .irq_priority = 7,
    .p = {0}
};

// DBG
usart_t usart7 = {
    .init = {
        .Instance = UART7,
        .Init.BaudRate = 115200,
        .Init.ClockPrescaler = 1,
        .Init.Mode = UART_MODE_TX_RX,
        .Init.OverSampling = UART_OVERSAMPLING_16,
        .Init.Parity = UART_PARITY_NONE,
        .Init.StopBits = UART_STOPBITS_1,
        .Init.WordLength = UART_WORDLENGTH_8B
    },
    .mode = USART_IDLE,
    .gpio_tx = &gpio_usart7_tx,
    .gpio_rx = &gpio_usart7_rx,
    .timeout = 100,
    .buf_size = 1024,
    .irq_priority = 9,
    .task_priority = 1,
    .p = {0}
};
