#include "cli.h"

gpio_cfg_t cli_tx = GPIO_USART7_TX;
gpio_cfg_t cli_rx = GPIO_USART7_RX;

usart_cfg_t cli_cfg = {UART7, 
                      &cli_tx, 
                      &cli_rx, 
                      115200, 20, 6, {NULL}};

int CLI_Init() {
    return USART_Init(&cli_cfg);
}

void retarget_put_char(uint8_t c) {
    USART_Transmit(&cli_cfg, &c, 1);
}

void UART7_IRQHandler(void) {
    USART_Handler(&cli_cfg);
}
