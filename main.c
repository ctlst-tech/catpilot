#include "stm32_base.h"
#include "stm32_drv.h"
#include "stm32_periph.h"

void Receive(void *pvParameters) {
    uint8_t data;
    USART_Receive(&cli_cfg, &data, 1);
    while(1) {
    }
}

void Print(void *pvParameters) {
    CLI_Init();
    while(1) {
        vTaskDelay(1000);
    }
}

void Polling(void *pvParameters) {
    uint8_t reg;
    ICM20602_Init();
    while(1) {
        vTaskDelay(1000);
        reg = ICM20602_ReadReg((uint8_t)0x75);
        printf("\nICM20602 WHOAMI = 0x%x\n", reg);
    }
}

int main(void) {
    HAL_Init();
    RCC_Init();
    xTaskCreate(Print, "Print", 512, NULL, 2, NULL);
    xTaskCreate(Polling, "Polling", 512, NULL, 2, NULL);
    xTaskCreate(Receive, "Receive", 512, NULL, 1, NULL);
    vTaskStartScheduler();
    while(1) {
    }
}
