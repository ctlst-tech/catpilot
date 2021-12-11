#include "stm32_conf.h"

void vThread1(void *pvParameters) {
    while(1) {
    }
}

int main(void) {
    RCC_Init();
    xTaskCreate(vThread1, "Thread1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();
    while(1) {
    }
}