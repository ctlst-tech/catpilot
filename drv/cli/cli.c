#include "cli.h"

gpio_cfg_t usart2_tx = PIN_USART2_TX;
gpio_cfg_t usart2_rx = PIN_USART2_RX;

usart_cfg_t usart2_cfg = {USART2, &usart2_tx, &usart2_rx, 115200, 20, 6, NULL};

int CLI_Init() {
    return USART_Init(&usart2_cfg);
}

void retarget_put_char(uint8_t c) {
    USART_Transmit(&usart2_cfg, &c, 1);
}

void USART2_IRQHandler(void) {
    USART_Handler(&usart2_cfg);
}
