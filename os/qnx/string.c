#include <string.h>
#include <malloc.h>

size_t strnlen(const char *s, size_t maxlen) {
    char *buf = calloc(maxlen + 1, sizeof(char));
    strncpy(buf, s, maxlen);
    size_t rv = strlen(buf);
    free(buf);
    return rv;
}

char *strndup(const char *s, size_t maxlen) {
    char *buf = calloc(maxlen + 1, sizeof(char));
    strncpy(buf, s, maxlen);
    return buf;
}
