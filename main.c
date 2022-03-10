#include "stm32_base.h"
#include "stm32_drv.h"
#include "stm32_periph.h"

void PX4IO(void *pvParameters) {
    PX4IO_Init();
    while(1) {
        vTaskDelay(2000);
        PX4IO_Run();
    }
}

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
