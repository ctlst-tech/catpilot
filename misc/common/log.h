#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#define LOG_MAX_LENGTH 255
#define LOG_MAX_MODULE_NAME 10
#define LOG_DEBUG_ENABLE 1

#define LOG_INFO_TYPE  0
#define LOG_WARN_TYPE  1
#define LOG_ERROR_TYPE 2
#define LOG_DEBUG_TYPE 3
#define LOG_EMPTY_TYPE 4

void log_module(uint8_t msg_type, char *module, char *s, ...);
void log_enable(bool enable);

#define WELCOME() printf("\n\n\n \t\t\t\tCATPILOT GO\n\n");

#define _LOG(type, module, s, ...) {    \
    log_module(type, module, s, ##__VA_ARGS__);\
}

#define LOG_INFO(MODULE, S, ...)    _LOG(LOG_INFO_TYPE, MODULE, S, ##__VA_ARGS__)
#define LOG_WARN(MODULE, S, ...)    _LOG(LOG_WARN_TYPE, MODULE, S, ##__VA_ARGS__)
#define LOG_ERROR(MODULE, S, ...)   _LOG(LOG_ERROR_TYPE, MODULE, S, ##__VA_ARGS__)

#ifdef LOG_DEBUG_ENABLE
#define LOG_DEBUG(MODULE, S, ...)   _LOG(LOG_DEBUG_TYPE, MODULE, S, ##__VA_ARGS__)
#endif

#ifndef LOG_DEBUG_ENABLE
#define LOG_DEBUG(MODULE, S, ...)   _LOG(LOG_EMPTY_TYPE, MODULE, S, ##__VA_ARGS__)
#endif