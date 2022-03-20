#include "sensors.h"
#include "sensors_conf.h"

void Sensors_Task(void *pvParameters) {
    ICM20602_Init();
    IST8310_Init();
    vTaskDelay(1);
    while(1) {
        ICM20602_Run();
        IST8310_Run();
        vTaskDelay(5);
    }
}

void Sensors_Start() {
    xTaskCreate(Sensors_Task, "Sensors", 512, NULL, SENSORS_TASK_PRIORITY, NULL);
}
