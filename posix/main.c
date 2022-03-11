#include "posix_base.h"

void Task() {
    while(1) {
        printf("\nFreeRTOS Posix port\n");
        vTaskDelay(1000);
    }
}

int main() {
    xTaskCreate(Task, "Task", 1024, NULL, 1, NULL);
    vTaskStartScheduler();
}
