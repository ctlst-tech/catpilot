#include "cli.h"

gpio_cfg_t usart7_tx = PIN_USART7_TX;
gpio_cfg_t usart7_rx = PIN_USART7_RX;

usart_cfg_t usart7_cfg = {UART7, &usart7_tx, &usart7_rx, 115200, 20, 6, {NULL}};

int CLI_Init() {
    return USART_Init(&usart7_cfg);
}

void retarget_put_char(uint8_t c) {
    USART_Transmit(&usart7_cfg, &c, 1);
}

void UART7_IRQHandler(void) {
    USART_Handler(&usart7_cfg);
}
