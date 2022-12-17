#include "service.h"

static service_t _service[SERVICE_MAX];
static uint32_t num;

void service(void *area) {
    service_t *service = (service_t *)area;
    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();
    LOG_INFO(service->name, "Service started");
    LOG_DEBUG(service->name, "Period = %u ms, priority = %u", service->period,
              service->priority);

    while (1) {
        if (service->period > 0) {
            xTaskDelayUntil(&xLastWakeTime, service->period);
        }
        service->handler(service->area);
    }
}

service_t *service_start(char *name, void *area, void (*handler)(void *area),
                         uint32_t period, uint32_t priority) {
    service_t *rv;

    if (name == NULL || strlen(name) > SERVICE_MAX_NAME) {
        LOG_ERROR(name, "Wrong name");
        return NULL;
    }

    if (handler == NULL) {
        LOG_ERROR(name, "Wrong handler");
        return NULL;
    }

    strncpy(_service[num].name, name, SERVICE_MAX_NAME);
    _service[num].handler = handler;
    _service[num].period = period / portTICK_PERIOD_MS;
    _service[num].priority = priority;
    _service[num].area = area;

    if (xTaskCreate(service, _service[num].name, SERVICE_MAX_STACK_SIZE,
                    &_service[num], _service[num].priority, NULL) != pdTRUE) {
        LOG_ERROR(name, "Service start error");
        return NULL;
    }

    rv = &_service[num];
    num++;

    return rv;
}
