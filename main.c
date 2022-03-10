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
    float t1, t2;
    ICM20602_Init();
    IST8310_Init();
    vTaskDelay(1);
    while(1) {
        t1 = xTaskGetTickCount();
        ICM20602_Run();
        IST8310_Run();
        t2 = xTaskGetTickCount();
    }
}

FATFS fs;
static FIL file;
static char buf[101000];
static uint32_t buf_length = 0;
static uint64_t all = 0;
static uint64_t bad_open = 0;
static uint64_t bad_write = 0;
static uint64_t bad_close = 0;

void FS(void *pvParameters) {
    FRESULT res;
    int length;
    UINT ptr;
    float t_now, t_last;
    float t1, t2, t3;

    res = f_mount(&fs, "0:", 1);

    length = sprintf(buf, "\ntime\tax\tay\taz\twx\twy\twz\tmagx\tmagy\tmagz\t\n");

    res = f_open(&file, "log.txt", FA_CREATE_ALWAYS | FA_WRITE);
    res = f_write(&file, buf, length, &ptr);
    res = f_close(&file);

    while(1) {
        t_now = xTaskGetTickCount();
        buf_length += sprintf((buf + buf_length), "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t\n", t_now,
            icm20602_fifo.accel_x[0], icm20602_fifo.accel_y[0], icm20602_fifo.accel_z[0],
            icm20602_fifo.gyro_x[0], icm20602_fifo.gyro_y[0], icm20602_fifo.gyro_z[0],
            ist8310_data.mag_x, ist8310_data.mag_y, ist8310_data.mag_z);

        if(buf_length > 100000) {
            all++;
            res = f_open(&file, "log.txt", FA_OPEN_APPEND | FA_WRITE);
            if(res != FR_OK) bad_open++;
            t1 = xTaskGetTickCount();

            all++;
            res = f_write(&file, buf, buf_length, &ptr);
            if(res != FR_OK) bad_write++;
            t2 = xTaskGetTickCount();

            all++;
            res = f_close(&file);
            if(res != FR_OK) bad_close++;
            t3 = xTaskGetTickCount();
            buf_length = 0;
        }
        vTaskDelay(4);
    }
}

void PX4IO(void *pvParameters) {
    PX4IO_Init();
    while(1) {
        vTaskDelay(2000);
        PX4IO_Run();
    }
}

int main(void) {
    // SCB_EnableDCache();
    // SCB_EnableICache();
    HAL_Init();
    RCC_Init();
    xTaskCreate(Echo, "Echo", 512, NULL, 1, NULL);
    xTaskCreate(Sensors, "Sensors", 512, NULL, 2, NULL);
    xTaskCreate(FS, "FileSystem", 1024, NULL, 3, NULL);
    xTaskCreate(PX4IO, "PX4IO exchange", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    vTaskStartScheduler();
    while(1) {
    }
}
