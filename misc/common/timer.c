#include "timer.h"

static tim_sem_id_t timer_[MAX_TIMERS];
static int id_ = 0;
static void Timer_Callback(TimerHandle_t timer);

int Timer_Create(const char *const name) {
    timer_[id_].id = id_;
    timer_[id_].tim = xTimerCreate(name, 1, pdFALSE, &timer_[id_], Timer_Callback);
    if(timer_[id_].tim == NULL) return TIMER_ERROR;
    timer_[id_].sem = xSemaphoreCreateBinary();
    if(timer_[id_].sem == NULL) return TIMER_ERROR;
    id_++;
    return (timer_[id_ - 1].id);
}

int Timer_Start(int timer_id, uint32_t period_ms) {
    int rv;
    int id = timer_id;

    if(xTimerIsTimerActive(timer_[id].tim)) {
        if(xSemaphoreTake(timer_[id].sem, 0)) {
            xTimerStop(timer_[id].tim, 10 / portTICK_PERIOD_MS);
            return TIMER_END;
        }
        return TIMER_WORK;
    } else {
        rv = xTimerChangePeriod(timer_[id].tim, 
                                period_ms / portTICK_PERIOD_MS, 
                                10 / portTICK_PERIOD_MS);
        if(rv == pdFALSE) return TIMER_ERROR;
        rv = xTimerStart(timer_[id].tim, 10 / portTICK_PERIOD_MS);
        if(rv == pdFALSE) return TIMER_ERROR;
        return TIMER_START;
    }
}

int Timer_Stop(int timer_id) {
    int id = timer_id;
    xTimerStop(timer_[id].tim, 10 / portTICK_PERIOD_MS);
    return 0;
}

static void Timer_Callback(TimerHandle_t timer) {
    tim_sem_id_t *timer_id;
    timer_id = (tim_sem_id_t *)pvTimerGetTimerID(timer);
    xSemaphoreGive(timer_id->sem);
}
