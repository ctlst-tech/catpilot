#include "devices.h"
#include "icm20602.h"
#include "icm20689.h"

int IMU_Init(void) {
    if(ICM20602_Init()) return -1;
    if(ICM20689_Init()) return -1;
    vTaskDelay(1);
    while(icm20602_state != ICM20602_FIFO_READ ||
          icm20689_state != ICM20689_FIFO_READ) {
            ICM20602_Run();
            ICM20689_Run();
            vTaskDelay(10);
    }
    return 0;
}

void IMU_Update(void) {
    ICM20602_Run();
    ICM20689_Run(); 
}
