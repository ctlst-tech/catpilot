#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"

typedef struct {
    spi_cfg_t spi;
    uint8_t data[255];
} icm20602_cfg_t;

int ICM20602_Init(void);
uint8_t ICM20602_ReadReg(uint8_t reg);
