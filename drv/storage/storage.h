#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"

typedef struct {
    sdio_cfg_t sdio;
} storage_cfg_t;

int Storage_Init();
int Storage_Read(uint8_t *pdata, uint32_t address, uint32_t num);
int Storage_Write(uint8_t *pdata, uint32_t address, uint32_t num);
int Storage_Status();
