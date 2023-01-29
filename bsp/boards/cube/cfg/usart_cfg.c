#include "core.h"
#include "hal.h"
#include "board.h"

// TELEM1
usart_t usart2 = {
    .name = "ttyS0",
    .alt_name = "TELEM1",
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
    .dma_tx = {0},
    .dma_rx = {0},
    .mode = USART_IDLE,
    .gpio_tx = &gpio_usart2_tx,
    .gpio_rx = &gpio_usart2_rx,
    .timeout = portMAX_DELAY,
    .buf_size = 512,
    .irq_priority = 10,
    .task_priority = 2,
    .p = {0}
};

// TELEM2
usart_t usart3 = {
    .name = "ttyS1",
    .alt_name = "TELEM2",
    .init = {
        .Instance = USART3,
        .Init.BaudRate = 57600,
        .Init.ClockPrescaler = 1,
        .Init.Mode = UART_MODE_TX_RX,
        .Init.OverSampling = UART_OVERSAMPLING_16,
        .Init.Parity = UART_PARITY_NONE,
        .Init.StopBits = UART_STOPBITS_1,
        .Init.WordLength = UART_WORDLENGTH_8B
    },
    .dma_tx = {0},
    .dma_rx = {0},
    .mode = USART_IDLE,
    .gpio_tx = &gpio_usart3_tx,
    .gpio_rx = &gpio_usart3_rx,
    .timeout = portMAX_DELAY,
    .buf_size = 512,
    .irq_priority = 10,
    .task_priority = 2,
    .p = {0}
};

// GPS1
usart_t usart4 = {
    .name = "ttyS2",
    .alt_name = "GPS1",
    .init = {
        .Instance = UART4,
        .Init.BaudRate = 115200,
        .Init.ClockPrescaler = 1,
        .Init.Mode = UART_MODE_TX_RX,
        .Init.OverSampling = UART_OVERSAMPLING_16,
        .Init.Parity = UART_PARITY_NONE,
        .Init.StopBits = UART_STOPBITS_1,
        .Init.WordLength = UART_WORDLENGTH_8B
    },
    .dma_tx = {
        .init = {
            .Instance = DMA1_Stream4,
            .Init.Request = DMA_REQUEST_UART4_TX,
            .Init.Direction = DMA_MEMORY_TO_PERIPH,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_LOW,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE
        },
        .irq_priority = 8
    },
    .dma_rx = {
        .init = {
            .Instance = DMA1_Stream5,
            .Init.Request = DMA_REQUEST_UART4_RX,
            .Init.Direction = DMA_PERIPH_TO_MEMORY,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_LOW,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE
        },
        .irq_priority = 8
    },
    .mode = USART_IDLE,
    .gpio_tx = &gpio_usart4_tx,
    .gpio_rx = &gpio_usart4_rx,
    .timeout = portMAX_DELAY,
    .buf_size = 512,
    .irq_priority = 8,
    .task_priority = 8,
    .p = {0}
};

// IO
usart_t usart6 = {
    .name = "ttyS3",
    .alt_name = "IO",
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
    .dma_tx = {
        .init = {
            .Instance = DMA1_Stream6,
            .Init.Request = DMA_REQUEST_USART6_TX,
            .Init.Direction = DMA_MEMORY_TO_PERIPH,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_LOW,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE
        },
        .irq_priority = 7
    },
    .dma_rx = {
        .init = {
            .Instance = DMA1_Stream7,
            .Init.Request = DMA_REQUEST_USART6_RX,
            .Init.Direction = DMA_PERIPH_TO_MEMORY,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_LOW,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE
        },
        .irq_priority = 7
    },
    .mode = USART_IDLE,
    .gpio_tx = &gpio_usart6_tx,
    .gpio_rx = &gpio_usart6_rx,
    .timeout = 100,
    .irq_priority = 7,
    .p = {0}
};

// DBG
usart_t usart7 = {
    .name = "ttyS4",
    .alt_name = "DBG",
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
    .dma_tx = {0},
    .dma_rx = {0},
    .mode = USART_IDLE,
    .gpio_tx = &gpio_usart7_tx,
    .gpio_rx = &gpio_usart7_rx,
    .timeout = portMAX_DELAY,
    .buf_size = 512,
    .irq_priority = 11,
    .task_priority = 1,
    .p = {0}
};

// GPS2
usart_t usart8 = {
    .name = "ttyS5",
    .alt_name = "GPS2",
    .init = {
        .Instance = UART8,
        .Init.BaudRate = 115200,
        .Init.ClockPrescaler = 1,
        .Init.Mode = UART_MODE_TX_RX,
        .Init.OverSampling = UART_OVERSAMPLING_16,
        .Init.Parity = UART_PARITY_NONE,
        .Init.StopBits = UART_STOPBITS_1,
        .Init.WordLength = UART_WORDLENGTH_8B
    },
    .dma_tx = {
        .init = {
            .Instance = DMA2_Stream4,
            .Init.Request = DMA_REQUEST_UART8_TX,
            .Init.Direction = DMA_MEMORY_TO_PERIPH,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_LOW,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE
        },
        .irq_priority = 8
    },
    .dma_rx = {
        .init = {
            .Instance = DMA2_Stream5,
            .Init.Request = DMA_REQUEST_UART8_RX,
            .Init.Direction = DMA_PERIPH_TO_MEMORY,
            .Init.PeriphInc = DMA_PINC_DISABLE,
            .Init.MemInc = DMA_MINC_ENABLE,
            .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
            .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
            .Init.Mode = DMA_NORMAL,
            .Init.Priority = DMA_PRIORITY_LOW,
            .Init.FIFOMode = DMA_FIFOMODE_DISABLE
        },
        .irq_priority = 8
    },
    .mode = USART_IDLE,
    .gpio_tx = &gpio_usart8_tx,
    .gpio_rx = &gpio_usart8_rx,
    .timeout = portMAX_DELAY,
    .buf_size = 512,
    .irq_priority = 8,
    .task_priority = 8,
    .p = {0}
};

usart_t *usart[6] = {
    &usart2,
    &usart3,
    &usart4,
    &usart6,
    &usart7,
    &usart8,
};
