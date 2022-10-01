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
//    TIM_Stop(tim_monitor);
//    TIM_Start(tim_monitor);
}

uint32_t Monitor_GetCounter(void) {
//    return TIM_GetTick(tim_monitor);
}

static char stat_buffer[1024];

void Monitor_Update(void) {
//    vTaskGetRunTimeStats(stat_buffer);
//    printf("%s", stat_buffer);
//    printf("--------------------------------------------------------\n");
}
