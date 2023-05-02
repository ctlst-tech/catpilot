#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <unistd.h>

#include "ctlst_sensors_imu.h"
#include "spiplr.h"
#include "ctlst_sensors.h"

#define PULSE_CODE_UPDATE_TIMER 4
#define PULSE_CODE_UPDATE_POLLER_IRQ 5

#define SCLK_CLOCK_DIVIDER 25

typedef struct {
    struct {
        uint32_t freq;
        uint32_t prio_delta;
        uint32_t period_us;
    } cfg;
    struct {
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        pthread_barrier_t start_barrier;
        int chid;
        int coid;
        int rcvid;
        struct _pulse p;
        struct sigevent timer_event;
        struct sigevent intr_event;
        int intr_id;
        int irq_cnt;
        int irq_cnt_init;
        int irq_spiplr_cnt;
    } sync;
    sens_onboard_adc_data_t adc_data;
} ctlst_sensors_imu_t;
