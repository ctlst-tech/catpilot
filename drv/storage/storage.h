#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"

typedef struct {
    sdio_cfg_t sdio;
} storage_cfg_t;

int Storage_Init();
