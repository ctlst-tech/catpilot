#include "log.h"
#include "func.h"
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>

static char *msg_types[4] = {
    "INFO",
    "WARN",
    "ERROR",
    "DEBUG"
};

static char *msg_color[5] = {
    "\x1b[32m",
    "\x1b[33m",
    "\x1b[31m",
    "\x1B[37m",
    "\x1b[0m",
};

void log_module(uint8_t msg_type, char *module, char *s, ...) {
    ssize_t length = 0;
    char string[LOG_MAX_LENGTH] = {};
    char module_alig[20] = {};
    struct timespec t;

    if(msg_type == LOG_EMPTY_TYPE) return;

    memset(module_alig, ' ', LOG_MAX_MODULE_NAME - 1);
    memcpy(module_alig, module, MIN(strlen(module), LOG_MAX_MODULE_NAME));

    clock_gettime(CLOCK_MONOTONIC, &t);

    length += sprintf(string + length, "%.3f\t", t.tv_sec + t.tv_nsec * 1e-9);
    length += sprintf(string + length, "%s%s%s\t",
                     msg_color[msg_type], msg_types[msg_type], msg_color[4]);
    length += sprintf(string + length, "%s\t", module_alig);

    va_list arg;
    va_start(arg, s);
    length += vsnprintf(string + length, MAX(LOG_MAX_LENGTH - length, 0), s, arg);
    va_end(arg);

    printf("%s\n", string);
}