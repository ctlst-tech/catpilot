/* C standard library includes. */
#include <stddef.h>
#include <string.h>

/* FreeRTOS+POSIX includes. */
#include "FreeRTOS_POSIX.h"
#include "FreeRTOS_POSIX/errno.h"
#include "FreeRTOS_POSIX/time.h"
#include "FreeRTOS_POSIX/utils.h"

void *__wrap_malloc(size_t size) {
    return pvPortMalloc(size);
}

void *__wrap_calloc(size_t nmemb, size_t size) {
    return pvPortMalloc(nmemb * size);
}

void __wrap_free(void *ptr) {
    return vPortFree(ptr);
}
