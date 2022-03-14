#include "logger.h"
#include "logger_conf.h"
#include <string.h>

static FATFS fs;
static FIL file;

static char buf[LOGGER_BUFFER_SIZE];

void Logger_Task() {
    FRESULT res;
    UINT ptr;
    uint32_t length;
    float t_now, t_last;
    uint32_t t0, t1, t2, t3;

    res = f_mount(&fs, "0:", 1);

    length = sprintf(buf, "time\tax\tay\taz\twx\twy\twz\tmagx\tmagy\tmagz\t\n");

    res = f_open(&file, "log.txt", FA_CREATE_ALWAYS | FA_WRITE);
    res = f_write(&file, buf, length, &ptr);
    res = f_close(&file);

    length = 0;

    while(1) {
        t_now = xTaskGetTickCount();
        t0 = xTaskGetTickCount();
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

        if(length > LOGGER_WRITE_SIZE) {
            res = f_open(&file, "log.txt", FA_OPEN_EXISTING | FA_WRITE);
            if(res) {
                vTaskDelay(0);
            }

            t1 = xTaskGetTickCount();

            for(uint32_t i = 0; i < length;) {
                if(length - i > LOGGER_BLOCK_SIZE) {
                    f_lseek(&file, f_size(&file));
                    res = f_write(&file, buf + i, LOGGER_BLOCK_SIZE, &ptr);

                    if(res) {
                        vTaskDelay(0);
                    }

                    i = i + LOGGER_BLOCK_SIZE;
                } else if(length == i) {
                    length = 0;
                } else {
                    length = length - i;
                    res = f_write(&file, buf + i, length, &ptr);

                    if(res) {
                        vTaskDelay(0);
                    }

                    length = 0;
                }
            }

            t2 = xTaskGetTickCount();

            res = f_close(&file);

            if(res) {
                vTaskDelay(0);
            }

            t3 = xTaskGetTickCount();

            if((t3 - t0) > 100U) {
                vTaskDelay(0);
            }

        } else {
            vTaskDelay(4);
        }
    }
}

void Logger_Start() {
    xTaskCreate(Logger_Task, "Logger", 10024, NULL, LOGGER_TASK_PRIORITY, NULL);
}
