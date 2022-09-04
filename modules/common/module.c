#include "module.h"
#include "string.h"

static module_t _module[MAX_MODULES];
static uint32_t index;

void Module_Task(void *param);

int Module_Start(char *name,
                 int (*init)(void),
                 void (*update)(void),
                 uint32_t period,
                 uint32_t priority) {
    int rv = 0;

    if(name == NULL || strlen(name) > MAX_MODULE_NAME) {
        LOG_ERROR(name, "Wrong name");
        return -1;
    }

    strcpy(name, &_module[index].name);
    _module[index].init = init;
    _module[index].update = update;
    _module[index].period = period;
    _module[index].priority = priority;

    rv = xTaskCreate(Module_Task, 
                    _module[index].name, 
                    MAX_MODULE_STACK_SIZE, 
                    &_module[index], 
                    _module[index].priority, 
                    NULL);

    if(rv != pdTRUE) {
        LOG_ERROR(name, "Task start error");
        return -1;
    }

    index++;

    return rv;
}

void Module_Task(void *param) {
    module_t *module = (module_t *)param;
    TickType_t xLastWakeTime;

    LOG_DEBUG(module->name, "Start task");

    if(module->init()) {
        LOG_DEBUG(module->name, "Init error");
    } else {
        LOG_DEBUG(module->name, "Init successful");
    }

    xLastWakeTime = xTaskGetTickCount();

    while(1) {
        module->update();
        xTaskDelayUntil(&xLastWakeTime, module->period);
    }
}
