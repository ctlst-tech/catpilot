#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <unistd.h>

#include "ctlst_sensors_imu.h"

struct ctlst_sensors_imu {
    uint32_t freq;
    uint32_t prio_delta;
    uint32_t period_us;
    struct {
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        pthread_barrier_t start_barrier;
    } sync;
    int chid;
    int coid;
};

struct ctlst_sensors_imu *imu = NULL;

void *ctlst_sensors_imu_exec_service(void *arg) {
    struct ctlst_sensors_imu *c = (struct ctlst_sensors_imu *)arg;
    if (ThreadCtl(_NTO_TCTL_IO, 0) == -1) {
        error(__func__, "ThreadCtl");
        return NULL;
    }

    struct sched_param param = {0};
    int policy = 0;
    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_curpriority = (param.sched_priority += c->prio_delta);
    int rv = pthread_setschedparam(pthread_self(), policy, &param);
    if (rv != EOK) {
        error(__func__, "pthread_setschedparam failed");
        return NULL;
    }

    c->chid = ChannelCreate_r(0);
    c->chid = ConnectAttach_r(0, 0, c->chid, 0, 0);

    pthread_barrier_wait(&c->sync.start_barrier);

    while (1) {
        printf("Doing polling\n");
        pthread_mutex_lock(&c->sync.mutex);
        // Operations with data
        pthread_mutex_unlock(&c->sync.mutex);
        pthread_cond_broadcast(&c->sync.cond);
        usleep(c->period_us);
    }
}

fspec_rv_t ctlst_sensors_imu_pre_exec_init(const ctlst_sensors_imu_params_t *p,
                                           ctlst_sensors_imu_state_t *state) {
    if (imu != NULL) {
        error("ctlst.sensors.imu exists");
        return fspec_rv_exists;
    }
    imu = calloc(sizeof(struct ctlst_sensors_imu), sizeof(char));
    if (imu == NULL) {
        return fspec_rv_no_memory;
    }
    imu->freq = p->freq;
    imu->prio_delta = p->prio_delta;
    imu->period_us = 1000000 / p->freq;

    int rv = pthread_mutex_init(&imu->sync.mutex, NULL);
    if (rv != EOK) {
        goto exit_fail;
    }
    rv = pthread_cond_init(&imu->sync.cond, NULL);
    if (rv != EOK) {
        goto exit_fail;
    }

    rv = pthread_barrier_init(&imu->sync.start_barrier, NULL, 2);
    if (rv != EOK) {
        goto exit_fail;
    }

    rv = pthread_create(NULL, NULL, ctlst_sensors_imu_exec_service, imu);
    if (rv != EOK) {
        goto exit_fail;
    }

    pthread_barrier_wait(&imu->sync.start_barrier);
    pthread_barrier_destroy(&imu->sync.start_barrier);

    return fspec_rv_ok;

exit_fail:
    free(imu);
    error(__func__, "ctlst_sensors_imu_pre_exec_init failed");
    return fspec_rv_system_err;
}

void ctlst_sensors_imu_exec(ctlst_sensors_imu_outputs_t *o,
                            const ctlst_sensors_imu_params_t *p,
                            ctlst_sensors_imu_state_t *state) {
    while(1) {
        pthread_mutex_lock(&imu->sync.mutex);
        pthread_cond_wait(&imu->sync.cond, &imu->sync.mutex);
        pthread_mutex_unlock(&imu->sync.mutex);
        printf("Doing posting\n");
    }
    return;
}
