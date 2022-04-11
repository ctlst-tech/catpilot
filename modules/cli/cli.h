#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"

int CLI_Init();

// For printf in serial port
#define __weak   __attribute__((weak))
int _write(int fd, char* ptr, int len);
void cli_put_char(uint8_t c);

// For work with streams
int cli_put(char c, struct __file * file);
int cli_get(struct __file * file);
