#ifndef SERVICE_H
#define SERVICE_H

#include <string.h>

#include "log.h"
#include "os.h"

#define SERVICE_MAX 10
#define SERVICE_MAX_NAME 16
#define SERVICE_MAX_STACK_SIZE configMINIMAL_STACK_SIZE

typedef struct {
    char name[SERVICE_MAX_NAME];
    void *area;
    void (*handler)(void *area);
    uint32_t period;
    uint32_t priority;
} service_t;

service_t *service_start(char *name, void *area, void (*handler)(void *area),
                         uint32_t period, uint32_t priority);

#endif  // SERVICE_H
