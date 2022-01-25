#include "stm32_base.h"
#include "stm32_drv.h"
#include "stm32_periph.h"
#include "icm20602_reg.h"

void Echo(void *pvParameters) {
    CLI_Init();
    while(1) {
        ICM20602_Statistics();
        vTaskDelay(100);
    }
}

void Sensors(void *pvParameters) {
    ICM20602_Init();
    IST8310_Init();
    while(1) {
        ICM20602_Run();
        IST8310_Probe();
    }
}

int main(void) {
    HAL_Init();
    RCC_Init();
    xTaskCreate(Echo, "Echo", 512, NULL, 1, NULL);
    xTaskCreate(Sensors, "Sensors", 512, NULL, 2, NULL);
    vTaskStartScheduler();
    while(1) {
    }
}
