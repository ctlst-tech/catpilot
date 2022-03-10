#include "logger.h"
#include "logger_conf.h"

FATFS fs;
static FIL file;

static char buf[LOGGER_BUFFER_SIZE + LOGGER_MAX_STRING];
static uint32_t length = 0;

void Logger_Task() {
    FRESULT res;
    UINT ptr;
    float t_now, t_last;
    float t1, t2, t3;

    res = f_mount(&fs, "0:", 1);

    length = sprintf(buf, "\ntime\tax\tay\taz\twx\twy\twz\tmagx\tmagy\tmagz\t\n");

    res = f_open(&file, "log.txt", FA_CREATE_ALWAYS | FA_WRITE);
    res = f_write(&file, buf, length, &ptr);
    res = f_close(&file);

    length = 0;

    while(1) {
        t_now = xTaskGetTickCount();
        length += sprintf((buf + length), "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t\n", t_now,
            icm20602_fifo.accel_x[0], icm20602_fifo.accel_y[0], icm20602_fifo.accel_z[0],
            icm20602_fifo.gyro_x[0], icm20602_fifo.gyro_y[0], icm20602_fifo.gyro_z[0],
            ist8310_data.mag_x, ist8310_data.mag_y, ist8310_data.mag_z);

        if(length > LOGGER_BUFFER_SIZE) {
            res = f_open(&file, "log.txt", FA_OPEN_APPEND | FA_WRITE);
            t1 = xTaskGetTickCount();

            res = f_write(&file, buf, length, &ptr);
            t2 = xTaskGetTickCount();

            res = f_close(&file);
            t3 = xTaskGetTickCount();
            length = 0;
        }
        vTaskDelay(4);
    }
}

void Logger_Start() {
    xTaskCreate(Logger_Task, "Logger", 1024, NULL, LOGGER_TASK_PRIORITY, NULL);
}
