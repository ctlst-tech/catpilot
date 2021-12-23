#include "stm32_base.h"
#include "stm32_drv.h"
#include "stm32_periph.h"

void vThread1(void *pvParameters) {
    int rv;
    rv = CLI_Init();
    while(1) {
        vTaskDelay(1000);
        printf("\nSerial Console Test\n");
    }
}

int main(void) {
    HAL_Init();
    RCC_Init();
    xTaskCreate(vThread1, "Thread1", 512, NULL, 1, NULL);
    vTaskStartScheduler();
    while(1) {
    }
}
