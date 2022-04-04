#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"

#define PX4IO_RC_CHANNELS 16
#define PX4IO_MAX_OUTPUT  8

typedef struct {
    usart_cfg_t usart;
} px4io_cfg_t;

typedef struct {
    uint32_t status;
    uint32_t arming;
    uint32_t vservo;
    uint32_t vrssi;
    uint32_t alarms;
    uint32_t rc[PX4IO_RC_CHANNELS];
    uint32_t outputs[PX4IO_MAX_OUTPUT];
    uint32_t protocol_version;
    uint32_t hardware_version;
    uint32_t max_actuators;
    uint32_t max_controls;
    uint32_t max_transfer;
    uint32_t max_rc_input;
} px4io_reg_t;

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

