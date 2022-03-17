#include "logger.h"
#include "logger_conf.h"
#include <string.h>

static FATFS fs;
static FIL file;

static QueueHandle_t LoggerQueue;
static char str_buf[LOGGER_BUFFER_SIZE];
static char wr_buf[LOGGER_WRITE_SIZE];

void Logger_Buffer_Task() {
    uint32_t length;
    float time;
    LoggerQueue = xQueueCreate(1, LOGGER_WRITE_SIZE);
    length = sprintf(str_buf, "time\tax\tay\taz\twx\twy\twz\tmagx\tmagy\tmagz\t\n");
    while(1) {
        time = xTaskGetTickCount();
        length += sprintf((str_buf + length), "%f\t", time);
        length += sprintf((str_buf + length), "%f\t", icm20602_fifo.accel_x[0]);
        length += sprintf((str_buf + length), "%f\t", icm20602_fifo.accel_y[0]);
        length += sprintf((str_buf + length), "%f\t", icm20602_fifo.accel_z[0]);
        length += sprintf((str_buf + length), "%f\t", icm20602_fifo.gyro_x[0]);
        length += sprintf((str_buf + length), "%f\t", icm20602_fifo.gyro_y[0]);
        length += sprintf((str_buf + length), "%f\t", icm20602_fifo.gyro_z[0]);
        length += sprintf((str_buf + length), "%f\t", ist8310_data.mag_x);
        length += sprintf((str_buf + length), "%f\t", ist8310_data.mag_y);
        length += sprintf((str_buf + length), "%f\t", ist8310_data.mag_z);
        length += sprintf((str_buf + length), "\n");
        if(length > LOGGER_WRITE_SIZE) {
            xQueueSend(LoggerQueue, str_buf, portMAX_DELAY);
            length = length - LOGGER_WRITE_SIZE;
            memcpy(str_buf, str_buf + LOGGER_WRITE_SIZE, length);
        }
    }
}

void Logger_Write_Task() {
    FRESULT res;
    uint32_t ptr;

    uint32_t t0, t1, t2, t3;

    res = f_mount(&fs, "0:", 1);
    res = f_open(&file, "log.txt", FA_CREATE_ALWAYS | FA_WRITE);
    res = f_close(&file);

    while(1) {
        if(xQueueReceive(LoggerQueue, wr_buf, portMAX_DELAY)) {
            t0 = xTaskGetTickCount();
            res = f_open(&file, "log.txt", FA_OPEN_APPEND | FA_WRITE);

            if(res) {
                vTaskDelay(0);
            }

            t1 = xTaskGetTickCount();
            res = f_write(&file, wr_buf, LOGGER_BLOCK_SIZE, &ptr);

            if(res) {
                vTaskDelay(0);
            }

            t2 = xTaskGetTickCount();
            res = f_close(&file);

            if(res) {
                vTaskDelay(0);
            }

            t3 = xTaskGetTickCount();

            printf("\nLogger: Statistics\n");
            printf("Logger: f_open  time = %lu\n", t1 - t0);
            printf("Logger: f_wirte time = %lu\n", t2 - t1);
            printf("Logger: f_close time = %lu\n", t3 - t2);
            printf("Logger: Total time   = %lu\n", t3 - t0);

            if((t3 - t0) > 100U) {
                vTaskDelay(0);
            }
        }
    }
}

void Logger_Start() {
    xTaskCreate(Logger_Write_Task, "Write to SD", 1024, NULL, LOGGER_WRITE_TASK_PRIORITY, NULL);
    xTaskCreate(Logger_Buffer_Task, "Bufferization", 1024, NULL, LOGGER_BUFFER_TASK_PRIORITY, NULL);
}
