#include "module.h"
#include "string.h"

static module_t _module[MAX_MODULES];
static uint32_t num;

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

    strcpy(_module[num].name, name);
    _module[num].init = init;
    _module[num].update = update;
    _module[num].period = period;
    _module[num].priority = priority;

    rv = xTaskCreate(Module_Task, 
                    _module[num].name, 
                    MAX_MODULE_STACK_SIZE, 
                    &_module[num], 
                    _module[num].priority, 
                    NULL);

    if(rv != pdTRUE) {
        LOG_ERROR(name, "Task start error");
        return -1;
    }

    num++;

    return rv;
}

void Module_Task(void *param) {
    module_t *module = (module_t *)param;
    TickType_t xLastWakeTime;

    if(module->init()) {
        LOG_ERROR(module->name, "Initialization error");
    } else {
        LOG_INFO(module->name, "Initialization successful");
    }

    xLastWakeTime = xTaskGetTickCount();

    while(1) {
        module->update();
        xTaskDelayUntil(&xLastWakeTime, module->period / portTICK_PERIOD_MS);
    }
}
