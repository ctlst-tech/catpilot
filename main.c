#include "stm32_base.h"
#include "stm32_drv.h"
#include "stm32_periph.h"
#include "icm20602_reg.h"

void Echo(void *pvParameters) {
    CLI_Init();
    while(1) {
        vTaskDelay(1000);
    }
}

void Gyro(void *pvParameters) {
    uint8_t high;
    uint8_t low;
    ICM20602_Init();
    while(1) {
        ICM20602_Run();
    }
}

int main(void) {
    HAL_Init();
    RCC_Init();
    xTaskCreate(Echo, "Echo", 512, NULL, 1, NULL);
    xTaskCreate(Gyro, "Gyro", 512, NULL, 2, NULL);
    vTaskStartScheduler();
    while(1) {
    }
}
