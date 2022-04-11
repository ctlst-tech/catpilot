#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"

int CLI_Init();
void CLI_Start();
void retarget_put_char(uint8_t c);
int cli_put(char c, struct __file * file);
// int cli_get(char c, struct __file * file);
