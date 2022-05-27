#include "logger.h"
#include "logger_conf.h"
#include <string.h>

static FATFS fs;
static FIL file;

static QueueHandle_t LoggerQueue;
static char str_buf[LOGGER_BUFFER_SIZE];
static char wr_buf[LOGGER_WRITE_SIZE];

static int frame_num;
static logger_frame_t *frame[LOGGER_MAX_FRAMES];

int Logger_AddFrame(char *name) {
    if(name == NULL) return -1;
    if(strlen(name) > LOGGER_FRAME_NAME_MAX_LENGTH) return -1;
    for(int i = 0; i < frame_num; i++) {
        if(!strcmp(name, frame[i]->name)) {
            return frame[i]->frame_id;
        }
    }
    int num = frame_num;
    frame[frame_num] = calloc(1, sizeof(logger_frame_t));
    strcpy(frame[frame_num]->name, name);
    frame[frame_num]->frame_id = frame_num;
    frame_num++;
    return num;
}

int Logger_DeleteFrame(char *name) {
    if(name == NULL) return -1;
    if(strlen(name) > LOGGER_FRAME_NAME_MAX_LENGTH) return -1;
    for(int i = 0; i < frame_num; i++) {
        if(!strcmp(name, frame[i]->name)) {
            int id = frame[i]->frame_id;
            free(frame[i]);
            frame_num--;
            return id;
        }
    }
}

int Logger_AddSignal(char *frame_name, char *signal_name) {
    if(frame_name == NULL) return -1;
    if(signal_name == NULL) return -1;
    if(strlen(signal_name) > LOGGER_SIGNAL_NAME_MAX_LENGTH) return -1;
    if(strlen(frame_name) > LOGGER_FRAME_NAME_MAX_LENGTH) return -1;
    for(int i = 0; i < frame_num; i++) {
        if(!strcmp(frame_name, frame[i]->name)) {
            for(int j = 0; j < frame[i]->signal_num; j++) {
                if(!strcmp(signal_name, frame[i]->signal[j]->name)) {
                    return (frame[i]->signal[j]->signal_id);
                }
            }
            int num = frame[i]->signal_num;
            frame[i]->signal[num] = calloc(1, sizeof(logger_signal_t));
            strcpy(frame[i]->signal[num]->name, signal_name);
            frame[i]->signal[num]->signal_id = num;
            frame[i]->signal_num++;
            return num;
        }
    }
    return -1;
}

int Logger_UpdateSignal(char *frame_name, char *signal_name, double value) {
    if(frame_name == NULL) return -1;
    if(signal_name == NULL) return -1;
    if(strlen(signal_name) > LOGGER_SIGNAL_NAME_MAX_LENGTH) return -1;
    if(strlen(frame_name) > LOGGER_FRAME_NAME_MAX_LENGTH) return -1;
    for(int i = 0; i < frame_num; i++) {
        if(!strcmp(frame_name, frame[i]->name)) {
            for(int j = 0; j < frame[i]->signal_num; j++) {
                if(!strcmp(signal_name, frame[i]->signal[j]->name)) {
                    frame[i]->signal[j]->value = value;
                    return frame[i]->signal[j]->signal_id;
                }
            }
        }
    }
    return -1;
}

void Logger_Buffer() {
    uint32_t length = 0;
    float time = 0;

    LoggerQueue = xQueueCreate(3, LOGGER_WRITE_SIZE);

    for(int i = 0; i < 20; i++) {
        length += sprintf(str_buf + length, "time%d\t", i);
        length += sprintf(str_buf + length, "ax%d\t", i);
        length += sprintf(str_buf + length, "ay%d\t", i);
        length += sprintf(str_buf + length, "az%d\t", i);
        length += sprintf(str_buf + length, "wx%d\t", i);
        length += sprintf(str_buf + length, "wy%d\t", i);
        length += sprintf(str_buf + length, "wz%d\t", i);
        length += sprintf(str_buf + length, "magx%d\t", i);
        length += sprintf(str_buf + length, "magy%d\t", i);
        length += sprintf(str_buf + length, "magz%d\t", i);
    }
    length += sprintf(str_buf + length, "\n");

    while(1) {
        time = xTaskGetTickCount();
        for(int i = 0; i < 20; i++) {
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
        }
        length += sprintf((str_buf + length), "\n");
        if(length > LOGGER_WRITE_SIZE) {
            xQueueSend(LoggerQueue, str_buf, portMAX_DELAY);
            length = length - LOGGER_WRITE_SIZE;
            memcpy(str_buf, str_buf + LOGGER_WRITE_SIZE, length);
        }
        time = xTaskGetTickCount();
        vTaskDelay(1);
    }
}

void Logger_Write() {
    FRESULT res;
    uint32_t ptr;

    uint32_t t0, t1, t2, t3;
    (void)t0;
    (void)t1;
    (void)t2;
    (void)t3;

    res = f_open(&file, "log.txt", FA_CREATE_ALWAYS | FA_WRITE);
    res = f_close(&file);

    res = f_open(&file, "log.txt", FA_OPEN_EXISTING | FA_WRITE);
    res = f_lseek(&file, f_size(&file));
    while(1) {
        if(xQueueReceive(LoggerQueue, wr_buf, portMAX_DELAY)) {
            t0 = xTaskGetTickCount();

            if(res) {
                vTaskDelay(0);
            }

            t1 = xTaskGetTickCount();
            res = f_write(&file, wr_buf, LOGGER_BLOCK_SIZE, &ptr);

            if(res) {
                vTaskDelay(0);
            }

            t2 = xTaskGetTickCount();

            if(res) {
                vTaskDelay(0);
            }

            t3 = xTaskGetTickCount();

            if((t3 - t0) > 100U) {
                vTaskDelay(0);
            }
            if(t0 > 120000) {
                res = f_close(&file);
                while(1);
            }
        }
    }
}

void Logger_Start() {
    xTaskCreate(Logger_Write, "Write to SD", 1024, NULL, LOGGER_WRITE_TASK_PRIORITY, NULL);
    xTaskCreate(Logger_Buffer, "Bufferization", 1024, NULL, LOGGER_BUFFER_TASK_PRIORITY, NULL);
}
