#include "logger.h"
#include "logger_conf.h"
#include <string.h>

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

int Logger_AddSignal(int frame_id, char *signal_name) {
    if(frame[frame_id] == NULL) return -1;
    if(signal_name == NULL) return -1;
    if(strlen(signal_name) > LOGGER_SIGNAL_NAME_MAX_LENGTH) return -1;
    for(int j = 0; j < frame[frame_id]->signal_num; j++) {
        if(!strcmp(signal_name, frame[frame_id]->signal[j]->name)) {
            return (frame[frame_id]->signal[j]->signal_id);
        }
    }
    int num = frame[frame_id]->signal_num;
    frame[frame_id]->signal[num] = calloc(1, sizeof(logger_signal_t));
    strcpy(frame[frame_id]->signal[num]->name, signal_name);
    frame[frame_id]->signal[num]->signal_id = num;
    frame[frame_id]->signal_num++;
    return num;
}

int Logger_UpdateSignal(int frame_id, int signal_id, double value) {
    if(frame[frame_id] == NULL) return -1;
    if(frame[frame_id]->signal[signal_id] == NULL) return -1;
    frame[frame_id]->signal[signal_id]->value = value;
    return signal_id;
}

int Logger_CreateFrameHeadString(int frame_id, char *head) {
    if(frame[frame_id] == NULL) return -1;
    int id = frame_id;
    int len = 0;
    for(int i = 0; i < frame[id]->signal_num; i++) {
        strcpy(head + len, frame[id]->signal[i]->name);
        len = strlen(head);
        head[len] = '\t';
        len++;
    }
    head[len] = '\n';
    len++;
    return len;
}

int Logger_CreateFrameValuesString(int frame_id, char *values) {
    if(frame[frame_id] == NULL) return -1;
    int id = frame_id;
    int len = 0;
    for(int i = 0; i < frame[id]->signal_num; i++) {
        len += sprintf(values + len, "%.lf\t", frame[id]->signal[i]->value);
    }
    values[len] = '\n';
    len++;
    return len;
}

int Logger_UpdateFrame(int frame_id) {
    int fd;
    int len;
    char head[LOGGER_MAX_FRAME_SIZE * LOGGER_SIGNAL_NAME_MAX_LENGTH] = {};
    char values[LOGGER_MAX_FRAME_SIZE * 5];
    char frame_path[LOGGER_FRAME_NAME_MAX_LENGTH * 2] = {};
    int mode = S_IRUSR | S_IWUSR | S_IXUSR |
               S_IRGRP | S_IWGRP | S_IXGRP |
               S_IROTH | S_IXOTH;

    len = Logger_CreateFrameHeadString(frame_id, head);
    sprintf(frame_path, "logs/%s.txt", frame[frame_id]->name);
    mkdir("logs", mode);
    fd = open(frame_path, O_RDWR | O_CREAT | O_TRUNC);
    write(fd, head, len);
    while(1) {
        fd = open(frame_path, O_RDWR | O_APPEND);
        len = Logger_CreateFrameValuesString(frame_id, values);
        write(fd, values, len);
        close(fd);
        usleep(1000000);
    }
    return 0;
}

void Logger_Start() {
}
