#include "devices.h"
#include "init.h"

int IMU_Init(void) {
    int rv;

    rv = ICM20689_Init(&spi1, 
                       &gpio_spi1_cs1, 
                       &exti_spi1_drdy1);
    if(rv) return -1;

    rv = ICM20602_Init(&spi1, 
                       &gpio_spi1_cs2, 
                       &exti_spi1_drdy2);
    if(rv) return -1;

    vTaskDelay(1);

    while(ICM20602_Operation() || ICM20689_Operation()) {
        ICM20602_Run();
        ICM20689_Run();
        vTaskDelay(2);
    }

    return 0;
}

void IMU_Update(void) {
    ICM20602_Run();
    ICM20689_Run(); 
}
