#include "FreeRTOS.h"
#include "croutine.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#define TIMER_OK    0
#define TIMER_ERROR -1
#define TIMER_START 1
#define TIMER_WORK  2
#define TIMER_END   3

#define MAX_TIMERS 16

typedef struct {
    TimerHandle_t tim;
    SemaphoreHandle_t sem;
    int id;
} tim_sem_id_t;

int Timer_Create(const char *const name);
int Timer_Start(int timer_id, uint32_t period_ms);
int Timer_Stop(int timer_id);
            