#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"

typedef struct {
    spi_cfg_t spi;
    uint8_t data[255];
} icm20602_cfg_t;

int ICM20602_Init();

void ICM20602_Run();
int ICM20602_Probe();

uint8_t ICM20602_ReadReg(uint8_t reg);
void ICM20602_WriteReg(uint8_t reg, uint8_t value);
void ICM20602_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits);
void ICM20602_Configure();
