#include "stm32_base.h"
#include "stm32_drv.h"
#include "stm32_periph.h"
#include "icm20602_reg.h"

void Echo(void *pvParameters) {
    CLI_Init();
    while(1) {
        // ICM20602_Statistics();
        vTaskDelay(100);
    }
}

void Sensors(void *pvParameters) {
    ICM20602_Init();
    IST8310_Init();
    vTaskDelay(1);
    while(1) {
        ICM20602_Run();
        IST8310_Run();
    }
}

void SDCARD(void *pvParameters) {
    SDCARD_Init();
    uint8_t data[2];
    uint8_t rec[2] = {};
    data[0] = 0x05;
    data[1] = 0x07;
    while(1) {
        SDCARD_Write(data, 0, 2);
        SDCARD_Read(rec, 0, 2);
        vTaskDelay(100);
    }
}

int main(void) {
    RCC_Init();
    HAL_Init();
    xTaskCreate(Echo, "Echo", 512, NULL, 1, NULL);
    xTaskCreate(Sensors, "Sensors", 512, NULL, 2, NULL);
    xTaskCreate(SDCARD, "SDCARD", 512, NULL, 3, NULL);
    vTaskStartScheduler();
    while(1) {
    }
}
