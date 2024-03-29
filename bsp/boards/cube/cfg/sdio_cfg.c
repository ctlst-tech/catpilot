#include "core.h"
#include "hal.h"
#include "board.h"

sdio_t sdio = {
    .cd = &gpio_sdmmc1_cd,
    .ck = &gpio_sdmmc1_ck,
    .cmd = &gpio_sdmmc1_cmd,
    .d0 = &gpio_sdmmc1_d0,
    .d1 = &gpio_sdmmc1_d1,
    .d2 = &gpio_sdmmc1_d2,
    .d3 = &gpio_sdmmc1_d3,
    .init.Instance = SDMMC1,
    .init.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING,
    .init.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE,
    .init.Init.BusWide = SDMMC_BUS_WIDE_4B,
    .init.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE,
    .init.Init.ClockDiv = 1,
    .irq_priority = 11,
    .timeout = portMAX_DELAY
};
