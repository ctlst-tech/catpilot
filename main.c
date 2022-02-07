#include "stm32_base.h"
#include "stm32_drv.h"
#include "stm32_periph.h"
#include "icm20602_reg.h"
#include "ff.h"

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

uint8_t data[512] = {};

void SDCARD(void *pvParameters) {
    FATFS fileSystem;
    FIL testFile;
    uint8_t testBuffer[16] = "SD write success";
    UINT testBytes;
    FRESULT res;

    fileSystem.pdrv = 1;

    res = f_mount(&fileSystem, "0:", 1);
    while(1) {
    }
}

int main(void) {
    RCC_Init();
    HAL_Init();
    xTaskCreate(Echo, "Echo", 512, NULL, 1, NULL);
    // xTaskCreate(Sensors, "Sensors", 512, NULL, 2, NULL);
    xTaskCreate(SDCARD, "SDCARD", 512, NULL, 3, NULL);
    vTaskStartScheduler();
    while(1) {
    }
}
