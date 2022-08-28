#include "imu.h"
#include "imu_conf.h"
#include "log.h"

void IMU_Task(void *pvParameters) {
    LOG_DEBUG("IMU", "Start task");
    ICM20602_Init();
    vTaskDelay(1);
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    while(icm20602_state != ICM20602_FIFO_READ) {
            ICM20602_Run();
            vTaskDelayUntil(&xLastWakeTime, IMU_TASK_PERIOD_MS);
    }

    LOG_DEBUG("IMU", "Start of measurements");

    while(1) {
        ICM20602_Run();
        vTaskDelayUntil(&xLastWakeTime, IMU_TASK_PERIOD_MS);
    }
}

void IMU_Start() {
    xTaskCreate(IMU_Task, "IMU", 512, NULL, IMU_TASK_PRIORITY, NULL);
}
