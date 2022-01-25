#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"
#include "const.h"

typedef struct {
    i2c_cfg_t i2c;
} ist8310_cfg_t;

int IST8310_Init();

uint8_t IST8310_ReadReg(uint8_t reg);
void IST8310_WriteReg(uint8_t reg, uint8_t value);

int IST8310_Probe();
