#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"

typedef struct {
    usart_cfg_t usart;
} px4io_cfg_t;

int PX4IO_Init();
void PX4IO_Run();
int PX4IO_Read(uint16_t address, uint16_t length);
int PX4IO_Write(uint16_t address, uint16_t *data, uint16_t length);
int PX4IO_ReadRegs(uint8_t page, uint8_t offset, uint8_t num);
int PX4IO_WriteRegs(uint8_t page, uint8_t offset, uint16_t *data, uint8_t num);
uint32_t PX4IO_ReadReg(uint8_t page, uint8_t offset);
int PX4IO_WriteReg(uint8_t page, uint8_t offset, uint16_t data);
int PX4IO_SetClearReg(uint8_t page, uint8_t offset, uint16_t setbits, uint16_t clearbits);

int PX4IO_SetArmingState();
int PX4IO_GetIOStatus();
int PX4IO_GetRC();
int PX4IO_SetPWM(uint16_t *outputs, uint16_t num);

