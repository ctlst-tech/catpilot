#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"

typedef struct {
    int fd;
    usart_cfg_t *usart;
} cli_cfg_t;

int CLI_Init(usart_cfg_t *usart);
int CLI_Put(char c, struct __file * file);
int CLI_Get(struct __file * file);
int CLI_EchoStart(void);
