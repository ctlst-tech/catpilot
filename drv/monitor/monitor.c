#include "monitor.h"

static char *device = "Monitor";

// Data strucutres
static tim_cfg_t *tim_monitor;

// Private functions

// Sync

// Public functions
int Monitor_Init(tim_cfg_t *tim) {
    if(tim == NULL) return -1;
    tim_monitor = tim;

    return 0;
}

void Monitor_StartTimer(void) {
    TIM_Stop(tim_monitor);
    TIM_Start(tim_monitor);
}

uint32_t Monitor_GetCounter(void) {
    return TIM_GetTick(tim_monitor);
}

static char stat_buffer[4096];

void Monitor_Update(void) {
    static int first_delay = 1;
    if(first_delay) vTaskDelay(15000);
    first_delay = 0;
    vTaskGetRunTimeStats(stat_buffer);
    printf("------------------------ Monitor ------------------------\n");
    printf("%s", stat_buffer);
    printf("---------------------------------------------------------\n\n");
}
