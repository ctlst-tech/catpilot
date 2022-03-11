#include "stm32_base.h"
#include "stm32_drv.h"
#include "stm32_periph.h"

#include "cli.h"
#include "io.h"
#include "logger.h"
#include "sensors.h"

int main(void) {
    HAL_Init();
    RCC_Init();

    CLI_Start();
    Sensors_Start();
    IO_Start();
    Logger_Start();

    vTaskStartScheduler();

    while(1) {
    }
}
