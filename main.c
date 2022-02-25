#include "stm32_base.h"
#include "stm32_drv.h"
#include "stm32_periph.h"
#include "ff.h"

void Echo(void *pvParameters) {
    CLI_Init();
    while(1) {
        ICM20602_Statistics();
        vTaskDelay(100);
    }
}

void Sensors(void *pvParameters) {
    ICM20602_Init();
    IST8310_Init();
    vTaskDelay(1);
    while(1) {
        ICM20602_Run();
        IST8310_Run();
    }
}

FATFS fs;
static FIL file;
char buf[255];

void FS(void *pvParameters) {
    FRESULT res;
    int length;
    UINT ptr;
    float t;

    res = f_mount(&fs, "0:", 1);

    length = sprintf(buf, "time\tax\tay\taz\twx\twy\twz\tmagx\tmagy\tmagz\t\n");

    res = f_open(&file, "log.txt", FA_CREATE_ALWAYS | FA_WRITE);
    res = f_write(&file, buf, length, &ptr);
    res = f_close(&file);
    vTaskDelay(1000);

    while(1) {
        t = xTaskGetTickCount() / 1000.f;
        length = sprintf(buf, "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t\n", t,
            icm20602_fifo.accel_x[0], icm20602_fifo.accel_y[0], icm20602_fifo.accel_z[0],
            icm20602_fifo.gyro_x[0], icm20602_fifo.gyro_y[0], icm20602_fifo.gyro_z[0],
            ist8310_data.mag_x, ist8310_data.mag_y, ist8310_data.mag_z);

        res = f_open(&file, "log.txt", FA_OPEN_APPEND | FA_WRITE);
        res = f_write(&file, buf, length, &ptr);
        res = f_close(&file);
        vTaskDelay(10);
    }
}

void PX4IO(void *pvParameters) {
    PX4IO_Init();
    while(1) {
        PX4IO_Run();
    }
}

int main(void) {
    // SCB_EnableDCache();
    // SCB_EnableICache();
    HAL_Init();
    RCC_Init();
    // xTaskCreate(Echo, "Echo", 512, NULL, 1, NULL);
    // xTaskCreate(Sensors, "Sensors", 512, NULL, 2, NULL);
    // xTaskCreate(FS, "FileSystem", 1024, NULL, 3, NULL);
    xTaskCreate(PX4IO, "PX4IO exchange", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();
    while(1) {
    }
}
