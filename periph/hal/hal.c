#include "hal.h"

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    uint8_t *crlf = "\r\n";
    if(huart->Instance == UART7) {
        if(*(huart->pRxBuffPtr) == '\r') {
            HAL_UART_Transmit_DMA(huart, crlf, 2);
        } else {
            HAL_UART_Transmit_DMA(huart, huart->pRxBuffPtr, 1);
        }
        HAL_UART_Receive_DMA(huart, huart->pRxBuffPtr, 1);
    }
}
