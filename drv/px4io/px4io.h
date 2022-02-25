#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"

typedef struct {
    usart_cfg_t usart;
} px4io_cfg_t;

int PX4IO_Init();
void PX4IO_Run();
int PX4IO_Write(uint8_t address, uint16_t *data, uint16_t length);
int PX4IO_Read(uint8_t address, uint16_t *data, uint16_t length);
int PX4IO_ReadReg(uint8_t page, uint8_t offset, uint16_t *data, uint8_t num);
