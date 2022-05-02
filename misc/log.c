#include "log.h"
#include <string.h>
#include <time.h>
#include <stdarg.h>

static char *msg_types[4] = {
    "INFO",
    "WARN",
    "ERROR",
    "DEBUG"
};

void log(uint8_t msg_type, char *module, char *s, ...) {
    ssize_t length = 0;
    char string[LOG_MAX_STRING_LENGTH];
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);

    length += sprintf(string + length, "%lld.%ld\t", t.tv_sec, t.tv_nsec / 1e6);
    length += sprintf(string + length, "%s\t", msg_types[msg_type]);
    length += sprintf(string + length, "%s\t", module);

    va_list arg;
    va_start(arg, s);
    length = (LOG_MAX_STRING_LENGTH - length > 0 ? length : 0);
    length += vsnprintf(string + length, length, s, arg);
    va_end(arg);
}
