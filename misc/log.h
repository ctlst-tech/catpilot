#pragma once
#include "stm32_base.h"
#include <stdarg.h>

#define LOG_MAX_STRING_LENGTH 100

#define LOG_INFO_TYPE  0
#define LOG_WARN_TYPE  1
#define LOG_ERROR_TYPE 2
#define LOG_DEBUG_TYPE 3

void log(uint8_t msg_type, char *module, char *s, ...);

#define _LOG(type, module, s, ...) {    \
    log(type, module, s, ##__VA_ARGS__);\
}

#define LOG_INFO(MODULE, S, ...)    _LOG(LOG_INFO_TYPE, MODULE, S, ...)
#define LOG_WARN(MODULE, S, ...)    _LOG(LOG_WARN_TYPE, MODULE, S, ...)
#define LOG_ERROR(MODULE, S, ...)   _LOG(LOG_ERROR_TYPE, MODULE, S, ...)
#define LOG_DEBUG(MODULE, S, ...)   _LOG(LOG_DEBUG_TYPE, MODULE, S, ...)
