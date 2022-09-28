#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"

#define MAX_MODULES 10
#define MAX_MODULE_NAME 16
#define MAX_MODULE_STACK_SIZE 512

typedef struct {
    char name[MAX_MODULE_NAME];
    int (*init)(void);
    void (*update)(void);
    uint32_t period;
    uint32_t priority;
} module_t;

int Module_Start(char *name,
                 int (*init)(void),
                 void (*update)(void),
                 uint32_t period,
                 uint32_t priority);
