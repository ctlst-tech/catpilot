#include "core.h"
#include "hal.h"
#include "gpio.h"
#include "exti.h"
#include "usart.h"
#include "board.h"

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
    .task_priority = 0,
    .buf_size = 0,
    .p = {0}
};
