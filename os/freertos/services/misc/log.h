#ifndef LOG_H
#define LOG_H

#include <errno.h>
#include <malloc.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define LOG_MAX_LENGTH 255
#define LOG_MAX_MODULE_NAME 10
#define LOG_DEBUG_ENABLE 1

#define LOG_INFO_TYPE 0
#define LOG_WARN_TYPE 1
#define LOG_ERROR_TYPE 2
#define LOG_DEBUG_TYPE 3
#define LOG_EMPTY_TYPE 4

enum log_type {
    LOG_TO_BUF = 0,
    LOG_TO_FILE = 1,
};

int log_init(const char *prefix, const char *path, enum log_type type,
             size_t buf_size);
void log_module(uint8_t msg_type, const char *module, const char *s, ...);
void log_enable(bool enable);

#define _LOG(type, module, s, ...) \
    { log_module(type, module, s, ##__VA_ARGS__); }

int log_print(int argc, char **argv);

#define LOG_INFO(MODULE, S, ...) _LOG(LOG_INFO_TYPE, MODULE, S, ##__VA_ARGS__)
#define LOG_WARN(MODULE, S, ...) _LOG(LOG_WARN_TYPE, MODULE, S, ##__VA_ARGS__)
#define LOG_ERROR(MODULE, S, ...) _LOG(LOG_ERROR_TYPE, MODULE, S, ##__VA_ARGS__)

#ifdef LOG_DEBUG_ENABLE
#define LOG_DEBUG(MODULE, S, ...) _LOG(LOG_DEBUG_TYPE, MODULE, S, ##__VA_ARGS__)
#endif

#ifndef LOG_DEBUG_ENABLE
#define LOG_DEBUG(MODULE, S, ...) _LOG(LOG_EMPTY_TYPE, MODULE, S, ##__VA_ARGS__)
#endif

#endif  // LOG_H
