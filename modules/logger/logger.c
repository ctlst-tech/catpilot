#include "logger.h"
#include "logger_conf.h"

FATFS fs;
static FIL file;

static char buf[LOGGER_BUFFER_SIZE + LOGGER_MAX_STRING];
static uint32_t length = 0;
static char buf_e[1024];

void Logger_Task() {
    FRESULT res;
    UINT ptr;
    float t_now, t_last;
    float t1, t2, t3;

    res = f_mount(&fs, "0:", 1);

    length = sprintf(buf, "time\tax\tay\taz\twx\twy\twz\tmagx\tmagy\tmagz\t\n");

    res = f_open(&file, "log.txt", FA_CREATE_ALWAYS | FA_WRITE);
    res = f_write(&file, buf, length, &ptr);
    res = f_close(&file);

    int j = 0;
    for(int i = 0; i < 1024; i++) {
        if(j <= 14) {
            buf_e[i] = '0' + j;
            j++;
        } else {
            j = 0;
            buf_e[i] = '\n';
        }
    }

    length = 0;

    while(1) {
        t_now = xTaskGetTickCount();
        length += sprintf((buf + length), "%f\t", t_now);
        length += sprintf((buf + length), "%f\t", icm20602_fifo.accel_x[0]);
        length += sprintf((buf + length), "%f\t", icm20602_fifo.accel_y[0]);
        length += sprintf((buf + length), "%f\t", icm20602_fifo.accel_z[0]);
        length += sprintf((buf + length), "%f\t", icm20602_fifo.gyro_x[0]);
        length += sprintf((buf + length), "%f\t", icm20602_fifo.gyro_y[0]);
        length += sprintf((buf + length), "%f\t", icm20602_fifo.gyro_z[0]);
        length += sprintf((buf + length), "%f\t", ist8310_data.mag_x);
        length += sprintf((buf + length), "%f\t", ist8310_data.mag_y);
        length += sprintf((buf + length), "%f\t", ist8310_data.mag_z);
        length += sprintf((buf + length), "\n");

        if(length > LOGGER_BUFFER_SIZE) {
            res = f_open(&file, "log.txt", FA_OPEN_APPEND | FA_WRITE);
            t1 = xTaskGetTickCount();

            for(size_t i = 0; i < 1024; i = i + 512) {
                res = f_write(&file, (buf_e + i), 512, &ptr);
            }
            t2 = xTaskGetTickCount();

            res = f_close(&file);
            t3 = xTaskGetTickCount();
            length = 0;
        }
        if(xTaskGetTickCount() > 60000U) {
            res = f_unmount("0:");
        }
        vTaskDelay(4);
    }
}

void Logger_Start() {
    xTaskCreate(Logger_Task, "Logger", 1024, NULL, LOGGER_TASK_PRIORITY, NULL);
}
