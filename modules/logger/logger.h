#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"
#include "drv.h"
#include "ff.h"

#define LOGGER_SIGNAL_NAME_MAX_LENGTH 32
#define LOGGER_FRAME_NAME_MAX_LENGTH 32
#define LOGGER_MAX_FRAME_SIZE 200
#define LOGGER_MAX_FRAMES 10

typedef struct {
    char name[LOGGER_SIGNAL_NAME_MAX_LENGTH];
    int signal_id;
    double value;
} logger_signal_t;

typedef struct {
    char name[LOGGER_FRAME_NAME_MAX_LENGTH];
    int frame_id;
    int fd;
    TaskHandle_t task;
    int priority;
    int period;
    double time;
    logger_signal_t *signal[LOGGER_MAX_FRAME_SIZE];
    int signal_num;
} logger_frame_t;

int Logger_Init();
int Logger_UpdateSignal(char *frame_name, char *signal_name, double value);
int Logger_Start(char *frame_name, int priority, int period);
