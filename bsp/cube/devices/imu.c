#include "devices.h"
#include "init.h"

int IMU_INT_Init(void) {
    int rv;

    rv = ICM20649_Init(&spi1, 
                       &gpio_spi1_cs1, 
                       &exti_spi1_drdy1,
                       0);

    if(rv) return -1;

    vTaskDelay(1);

    while(ICM20649_Operation()) {
        ICM20649_Run();
        vTaskDelay(2);
    }

    return 0;
}

void IMU_INT_Update(void) {
    ICM20649_Run();
}

int IMU_EXT_Init(void) {
    int rv;

    rv = ICM20948_Init(&spi4, 
                       &gpio_spi4_cs1, 
                       NULL,
                       NULL,
                       0,
                       0);

    if(rv) return -1;

    rv = ICM20602_Init(&spi4, 
                       &gpio_spi4_cs2, 
                       NULL);
    if(rv) return -1;

    vTaskDelay(1);

    while(ICM20602_Operation() && ICM20948_Operation()) {
        ICM20602_Run();
        ICM20948_Run();
        vTaskDelay(2);
    }

    return 0;
}

void IMU_EXT_Update(void) {
    ICM20602_Run();
    ICM20948_Run();
}
