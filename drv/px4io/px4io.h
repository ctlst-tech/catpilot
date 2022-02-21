#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"

typedef struct {
    usart_cfg_t usart;
} px4io_cfg_t;

int PX4IO_Init();
int PX4IO_Write(uint8_t address, uint8_t *data, uint8_t length);
int PX4IO_Read(uint8_t address, uint8_t *data, uint8_t length);
