#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"
#include "drv.h"
#include "ff.h"

#define LOGGER_SIGNAL_NAME_MAX_LENGTH 32
#define LOGGER_FRAME_NAME_MAX_LENGTH 32
#define LOGGER_MAX_FRAME_SIZE 200
#define LOGGER_MAX_FRAMES 10

void Logger_Start();

typedef struct {
    char name[LOGGER_SIGNAL_NAME_MAX_LENGTH];
    int signal_id;
    double value;
} logger_signal_t;

typedef struct {
    char name[LOGGER_FRAME_NAME_MAX_LENGTH];
    int frame_id;
    int signal_num;
    logger_signal_t *signal[LOGGER_MAX_FRAME_SIZE];
} logger_frame_t;
