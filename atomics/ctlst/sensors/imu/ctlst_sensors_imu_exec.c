#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <unistd.h>

#include "ctlst_sensors.h"
#include "ctlst_sensors_imu.h"
#include "ctlst_sensors_imu_data_types.h"
#include "error.h"
#include "spiplr.h"

static void ctlst_sensors_imu_update(ctlst_sensors_imu_t *imu);
static void ctlst_sensors_imu_process(ctlst_sensors_imu_outputs_t *o,
                                      ctlst_sensors_imu_t *imu);
static void ctlst_sensors_imu_print(ctlst_sensors_imu_outputs_t *o,
                                    ctlst_sensors_imu_t *imu);

static ctlst_sensors_imu_t *imu = NULL;

void *ctlst_sensors_imu_exec_service(void *arg) {
    ctlst_sensors_imu_t *c = (ctlst_sensors_imu_t *)arg;
    if (ThreadCtl(_NTO_TCTL_IO, 0) == -1) {
        dbg_msg("ThreadCtl failed");
        return NULL;
    }

    struct sched_param param = {0};
    int policy = 0;
    pthread_getschedparam(pthread_self(), &policy, &param);

    c->sync.chid = ChannelCreate_r(0);
    c->sync.coid = ConnectAttach_r(0, 0, c->sync.chid, 0, 0);

    pthread_barrier_wait(&c->sync.start_barrier);

    fcnav_init(c->cfg.period_us);
    ssd_fcnav_startup_diag();

    SIGEV_PULSE_INIT(&c->sync.intr_event, c->sync.coid,
                     param.sched_priority + 1, PULSE_CODE_UPDATE_POLLER_IRQ, 0);
    c->sync.intr_id = InterruptAttachEvent_r(SPIPLR_IRQ, &c->sync.intr_event,
                                             _NTO_INTR_FLAGS_TRK_MSK);
    fcnav_irq_en();
    fcnav_poll_start();
    fcnav_read_status();

    InterruptEnable();

    while (1) {
        c->sync.rcvid =
            MsgReceive_r(c->sync.chid, &c->sync.p, sizeof(struct _pulse), NULL);
        if (c->sync.rcvid > 0) continue;
        if (c->sync.rcvid < 0) {
            dbg_msg("MsgReceive failed");
            return NULL;
        }

        switch (c->sync.p.code) {
            case PULSE_CODE_UPDATE_POLLER_IRQ:
                fcnav_read_status();
                fcnav_reset_status();
                ctlst_sensors_imu_update(c);
                fcnav_irq_cnt_mon();
                if (c->sync.intr_id > 0) {
                    InterruptUnmask(SPIPLR_IRQ, c->sync.intr_id);
                }
                break;

            default:
                dbg_msg("Unknown code");
                break;
        }
    }
}

fspec_rv_t ctlst_sensors_imu_pre_exec_init(const ctlst_sensors_imu_params_t *p,
                                           ctlst_sensors_imu_state_t *state) {
    if (imu != NULL) {
        dbg_msg("ctlst.sensors.imu module exists");
        return fspec_rv_exists;
    }
    imu = calloc(sizeof(ctlst_sensors_imu_t), sizeof(char));
    if (imu == NULL) {
        return fspec_rv_no_memory;
    }
    imu->cfg.freq = p->freq;
    imu->cfg.period_us = 1000000 / p->freq;

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
    dbg_msg("ctlst_sensors_imu_pre_exec_init failed");
    return fspec_rv_system_err;
}

void ctlst_sensors_imu_exec(ctlst_sensors_imu_outputs_t *o,
                            const ctlst_sensors_imu_params_t *p,
                            ctlst_sensors_imu_state_t *state) {
    ctlst_sensors_imu_process(o, imu);
    // ctlst_sensors_imu_print(o, imu);
    return;
}

static void ctlst_sensors_imu_update(ctlst_sensors_imu_t *imu) {
    pthread_mutex_lock(&imu->sync.mutex);
    ssd_fcnav_raw_to_adc_data(NULL, &imu->adc_data);
    pthread_mutex_unlock(&imu->sync.mutex);
    pthread_cond_broadcast(&imu->sync.cond);
}

#define OFFSET_AND_SCALE(val, off, scale) \
    OFFSET_AND_SCALE_DIR(val, off, scale, 1)

#define OFFSET_AND_SCALE_DIR(val, off, scale, dir) \
    ((((val) - (off)) * (scale)) * dir)

static void ctlst_sensors_imu_process(ctlst_sensors_imu_outputs_t *o,
                                      ctlst_sensors_imu_t *imu) {
    pthread_mutex_lock(&imu->sync.mutex);
    pthread_cond_wait(&imu->sync.cond, &imu->sync.mutex);

    o->ax = OFFSET_AND_SCALE_DIR(imu->adc_data.Ax, ADIS1600X_OFFSET,
                                 ADIS16006_SCALE, DIRECTION_FORWARD);
    o->ay = OFFSET_AND_SCALE_DIR(imu->adc_data.Ay, ADIS1600X_OFFSET,
                                 ADIS16006_SCALE, DIRECTION_FORWARD);
    o->az = OFFSET_AND_SCALE_DIR(imu->adc_data.Az, ADIS1600X_OFFSET,
                                 ADIS16006_SCALE, DIRECTION_REVERSED);

    o->wx = OFFSET_AND_SCALE_DIR(imu->adc_data.Gx, 0, ADXRS450_SCALE,
                                 DIRECTION_FORWARD);
    o->wy = OFFSET_AND_SCALE_DIR(imu->adc_data.Gy, 0, ADXRS450_SCALE,
                                 DIRECTION_REVERSED);
    o->wz = OFFSET_AND_SCALE_DIR(imu->adc_data.Gz, 0, ADXRS450_SCALE,
                                 DIRECTION_FORWARD);

    o->wx = OFFSET_AND_SCALE_DIR(imu->adc_data.Gx, 0, ADXRS450_SCALE,
                                 DIRECTION_FORWARD);
    o->wy = OFFSET_AND_SCALE_DIR(imu->adc_data.Gy, 0, ADXRS450_SCALE,
                                 DIRECTION_REVERSED);
    o->wz = OFFSET_AND_SCALE_DIR(imu->adc_data.Gz, 0, ADXRS450_SCALE,
                                 DIRECTION_FORWARD);

    pthread_mutex_unlock(&imu->sync.mutex);
}

static void ctlst_sensors_imu_print(ctlst_sensors_imu_outputs_t *o,
                                    ctlst_sensors_imu_t *imu) {
    pthread_mutex_lock(&imu->sync.mutex);
    printf("\n");
    printf("Ax = %f\n", o->ax);
    printf("Ay = %f\n", o->ay);
    printf("Az = %f\n", o->az);
    printf("Wx = %f\n", o->wx);
    printf("Wy = %f\n", o->wy);
    printf("Wz = %f\n", o->wz);
    pthread_mutex_unlock(&imu->sync.mutex);
}
