#include "devices.h"
#include "init.h"

int IMU_Init(void) {
    int rv;

    rv = ICM20602_Init(&spi4, 
                       &gpio_spi4_cs2, 
                       NULL);
    if(rv) return -1;

    vTaskDelay(1);

    while(ICM20602_Operation()) {
        ICM20602_Run();
        vTaskDelay(2);
    }

    return 0;
}

void IMU_Update(void) {
    ICM20602_Run();
}
