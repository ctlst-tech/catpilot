#include "monitor.h"

static char *device = "Monitor";

// Data strucutres
static tim_t *tim_monitor;

// Private functions

// Sync

static int monitor_init = 0;
static int fd;

// Public functions
int Monitor_Init(tim_t *tim) {
    if(tim == NULL) return -1;
    tim_monitor = tim;
    TIM_Stop(tim_monitor);
    TIM_Start(tim_monitor);
    monitor_init = 1;
    fd = open("/dev/cli", O_RDWR);

    return 0;
}

void Monitor_StartTimer(void) {
    if(!monitor_init) return;
    TIM_Start(tim_monitor);
}

uint32_t Monitor_GetCounter(void) {
    if(!monitor_init) return 0;
    return TIM_GetTick(tim_monitor);
}

static char stat_buffer[1024];

void Monitor_Update(void) {
    vTaskGetRunTimeStats(stat_buffer);
    printf("\n\n");
    write(fd, stat_buffer, strlen(stat_buffer));
}
