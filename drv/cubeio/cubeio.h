#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"

typedef struct {
    usart_cfg_t *usart;
} cubeio_cfg_t;

enum cubeio_state_t {
    CubeIO_RESET,
    CubeIO_CONF,
    CubeIO_OPERATION,
    CubeIO_FAIL,
};

int CubeIO_Init(usart_cfg_t *usart);
int CubeIO_Operation(void);
void CubeIO_Run(void);
