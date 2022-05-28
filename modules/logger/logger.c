#include "logger.h"
#include "logger_conf.h"
#include <string.h>

static SemaphoreHandle_t rw_mutex;
static SemaphoreHandle_t addsig_mutex;

static QueueHandle_t LoggerQueue;
static char write_buf[LOGGER_WRITE_SIZE];

static int frame_num;
static logger_frame_t *frame[LOGGER_MAX_FRAMES];

int Logger_Init() {
    if(rw_mutex == NULL) rw_mutex = xSemaphoreCreateMutex();
    if(addsig_mutex == NULL) addsig_mutex = xSemaphoreCreateMutex();
    return 0;
}

int Logger_FindFrame(char *name) {
    for(int i = 0; i < frame_num; i++) {
        if(!strcmp(name, frame[i]->name)) {
            return frame[i]->frame_id;
        }
    }
    return -1;
}

int Logger_FindSignal(int frame_id, char *signal_name) {
    if(frame[frame_id] == NULL) return -1;
    for(int j = 0; j < frame[frame_id]->signal_num; j++) {
        if(!strcmp(signal_name, frame[frame_id]->signal[j]->name)) {
            return (frame[frame_id]->signal[j]->signal_id);
        }
    }
    return -1;
}

int Logger_AddFrame(char *name) {
    if(name == NULL) return -1;
    if(strlen(name) > LOGGER_FRAME_NAME_MAX_LENGTH) return -1;
    int frame_id = Logger_FindFrame(name);
    if(frame_id >= 0) return frame_id;
    int num = frame_num;
    frame[frame_num] = calloc(1, sizeof(logger_frame_t));
    strcpy(frame[frame_num]->name, name);
    frame[frame_num]->frame_id = frame_num;
    frame_num++;
    return num;
}

int Logger_AddSignal(int frame_id, char *signal_name) {
    if(frame[frame_id] == NULL) return -1;
    if(signal_name == NULL) return -1;
    if(strlen(signal_name) > LOGGER_SIGNAL_NAME_MAX_LENGTH) return -1;
    int signal_id = Logger_FindSignal(frame_id, signal_name);
    if(signal_id >= 0) return signal_id;
    if(!xSemaphoreTake(addsig_mutex, 0)) return -1;
    int num = frame[frame_id]->signal_num;
    frame[frame_id]->signal[num] = calloc(1, sizeof(logger_signal_t));
    strcpy(frame[frame_id]->signal[num]->name, signal_name);
    frame[frame_id]->signal[num]->signal_id = num;
    frame[frame_id]->signal_num++;
    xSemaphoreGive(addsig_mutex);
    return num;
}

int Logger_UpdateSignal(char *frame_name, char *signal_name, double value) {
    int frame_id = Logger_FindFrame(frame_name);
    if(frame_id < 0) frame_id = Logger_AddFrame(frame_name);
    int signal_id = Logger_FindSignal(frame_id, signal_name);
    if(signal_id < 0) signal_id = Logger_AddSignal(frame_id, signal_name);
    xSemaphoreTake(rw_mutex, portMAX_DELAY);
    frame[frame_id]->signal[signal_id]->value = value;
    xSemaphoreGive(rw_mutex);
    return signal_id;
}

int Logger_CreateFrameHeadString(int frame_id, char *buf) {
    if(frame[frame_id] == NULL) return -1;
    int id = frame_id;
    int len = 0;
    len = sprintf(buf, "time\t");
    for(int i = 0; i < frame[id]->signal_num; i++) {
        strcpy(buf + len, frame[id]->signal[i]->name);
        len = strlen(buf);
        buf[len] = '\t';
        len++;
    }
    buf[len] = '\n';
    len++;
    return len;
}

int Logger_CreateFrameValuesString(int frame_id, char *buf) {
    if(frame[frame_id] == NULL) return -1;
    int id = frame_id;
    int len = 0;
    len = sprintf(buf, "%lf\t", frame[frame_id]->time);
    for(int i = 0; i < frame[id]->signal_num; i++) {
        len += sprintf(buf + len, "%lf\t", frame[id]->signal[i]->value);
    }
    buf[len] = '\n';
    len++;
    return len;
}

int Logger_CreateFile(int frame_id) {
    if(frame[frame_id] == NULL) return -1;
    int fd;
    int mode = S_IRUSR | S_IWUSR | S_IXUSR |
               S_IRGRP | S_IWGRP | S_IXGRP |
               S_IROTH | S_IWOTH | S_IROTH;
    char frame_path[LOGGER_FRAME_NAME_MAX_LENGTH * 2] = {};
    mkdir("logs", mode);
    sprintf(frame_path, "logs/%s.txt", frame[frame_id]->name);
    fd = open(frame_path, O_RDWR | O_CREAT | O_TRUNC);
    return fd;
}

void Logger_FrameTask(void *frame_id) {
    int len;
    char *head;
    char *buf;
    int id = *((int *)frame_id);
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    int fd = Logger_CreateFile(id);
    xSemaphoreTake(addsig_mutex, portMAX_DELAY);
    head = calloc(1, frame[id]->signal_num * (LOGGER_SIGNAL_NAME_MAX_LENGTH + 1));
    len = Logger_CreateFrameHeadString(id, head);
    write(fd, head, len);
    free(head);
    buf = write_buf;
    while(1) {
        xSemaphoreTake(rw_mutex, portMAX_DELAY);
        frame[id]->time = xTaskGetTickCount() / 1000.f;
        len = Logger_CreateFrameValuesString(id, buf);
        write(fd, buf, len);
        sync();
        xSemaphoreGive(rw_mutex);
        vTaskDelayUntil(&xLastWakeTime, frame[id]->period);
    }
}

int Logger_Start(char *frame_name, int priority, int period) {
    int frame_id = Logger_FindFrame(frame_name);
    if(frame_id < 0) return -1;
    if(frame[frame_id] == NULL) return -1;
    if(frame[frame_id]->signal_num < 1) return -1;
    if(frame[frame_id]->task != NULL) return -1;
    frame[frame_id]->priority = priority;
    frame[frame_id]->period = period;
    xTaskCreate(Logger_FrameTask,
                "Start",
                LOGGER_FRAME_STACK_SIZE,
                &(frame[frame_id]->frame_id),
                priority,
                &(frame[frame_id]->task));
    return 0;
}
