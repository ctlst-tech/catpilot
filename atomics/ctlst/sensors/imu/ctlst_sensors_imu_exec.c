#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <unistd.h>

#include "ctlst_sensors_imu.h"
#include "ctlst_sensors_imu_data_types.h"
#include "error.h"
#include "spiplr.h"

static int ctlst_sensors_imu_spiplr_init(ctlst_sensors_imu_t *c);
static void ctlst_sensors_imu_update(ctlst_sensors_imu_t *imu);
static void ctlst_sensors_imu_spiplr_irq_cnt(ctlst_sensors_imu_t *imu);

ctlst_sensors_imu_t *imu = NULL;

void *ctlst_sensors_imu_exec_service(void *arg) {
    ctlst_sensors_imu_t *c = (ctlst_sensors_imu_t *)arg;
    if (ThreadCtl(_NTO_TCTL_IO, 0) == -1) {
        error("%s: ThreadCtl", __func__);
        return NULL;
    }

    struct sched_param param = {0};
    int policy = 0;
    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_curpriority = (param.sched_priority += c->cfg.prio_delta);
    int rv = pthread_setschedparam(pthread_self(), policy, &param);
    if (rv != EOK) {
        error("%s: pthread_setschedparam failed", __func__);
        return NULL;
    }

    c->sync.chid = ChannelCreate_r(0);
    c->sync.coid = ConnectAttach_r(0, 0, c->sync.chid, 0, 0);

    pthread_barrier_wait(&c->sync.start_barrier);

    ctlst_sensors_imu_spiplr_init(c);
    SIGEV_PULSE_INIT(&c->sync.intr_event, c->sync.coid,
                     param.sched_priority + 1, PULSE_CODE_UPDATE_POLLER_IRQ, 0);
    c->sync.intr_id = InterruptAttachEvent_r(SPIPLR_IRQ, &c->sync.intr_event,
                                             _NTO_INTR_FLAGS_TRK_MSK);
    spiplr_irq_poll_rdy_enable(&c->spiplr_instance);
    spiplr_poll_start(&c->spiplr_instance);
    uint32_t st;
    spiplr_get_status(&c->spiplr_instance, &st);

    InterruptEnable();

    while (1) {
        c->sync.rcvid =
            MsgReceive_r(c->sync.chid, &c->sync.p, sizeof(struct _pulse), NULL);
        if (c->sync.rcvid > 0) continue;
        if (c->sync.rcvid < 0) {
            error("%s: MsgReceive failed", __func__);
            return NULL;
        }

        switch (c->sync.p.code) {
            case PULSE_CODE_UPDATE_TIMER:
                ctlst_sensors_imu_update(c);
                break;

            case PULSE_CODE_UPDATE_POLLER_IRQ:
                spiplr_get_status(&c->spiplr_instance, &st);
                spiplr_reset_status(&c->spiplr_instance);
                ctlst_sensors_imu_update(c);
                ctlst_sensors_imu_spiplr_irq_cnt(c);
                if (c->sync.intr_id > 0) {
                    InterruptUnmask(SPIPLR_IRQ, c->sync.intr_id);
                }
                break;

            default:
                error("%s: Unknown code", __func__);
                break;
        }
    }
}

fspec_rv_t ctlst_sensors_imu_pre_exec_init(const ctlst_sensors_imu_params_t *p,
                                           ctlst_sensors_imu_state_t *state) {
    if (imu != NULL) {
        error("%s: ctlst.sensors.imu exists", __func__);
        return fspec_rv_exists;
    }
    imu = calloc(sizeof(ctlst_sensors_imu_t), sizeof(char));
    if (imu == NULL) {
        return fspec_rv_no_memory;
    }
    imu->cfg.freq = p->freq;
    imu->cfg.prio_delta = p->prio_delta;
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
    error("%s: ctlst_sensors_imu_pre_exec_init failed", __func__);
    return fspec_rv_system_err;
}

void ctlst_sensors_imu_exec(ctlst_sensors_imu_outputs_t *o,
                            const ctlst_sensors_imu_params_t *p,
                            ctlst_sensors_imu_state_t *state) {
    while (1) {
        pthread_mutex_lock(&imu->sync.mutex);
        pthread_cond_wait(&imu->sync.cond, &imu->sync.mutex);
        *o = imu->o;
        pthread_mutex_unlock(&imu->sync.mutex);
    }
    return;
}

static void ctlst_sensors_imu_update(ctlst_sensors_imu_t *imu) {
    pthread_mutex_lock(&imu->sync.mutex);
    pthread_mutex_unlock(&imu->sync.mutex);
    pthread_cond_broadcast(&imu->sync.cond);
    printf("%s \n", __func__);
}

static void ctlst_sensors_imu_init_cfg_array(uint32_t *cfg_vector) {
    spiplr_cfg_word_t cfg_word;

    int i = 0;
    int j;

    cfg_word.reserved = 0;

    cfg_word.sclk_divider = 10;
    cfg_word.tr_byte_num = 1;
    cfg_word.cs_vector = 0xFF;  // 0xF0;
    cfg_word.lsb_first = 0;
    cfg_word.clk_cpha = 0;
    cfg_word.clk_cpol = 0;
    cfg_word.clk_delay = 0;

    cfg_vector[i++] = *(uint32_t *)&cfg_word;  // for ID

    cfg_word.sclk_divider = 6;
    cfg_word.tr_byte_num = 1;
    cfg_word.cs_vector = 0xF1;
    cfg_word.lsb_first = 0;
    cfg_word.clk_cpha = 0;
    cfg_word.clk_cpol = 0;
    cfg_word.clk_delay = 0;

    cfg_vector[i++] = *(uint32_t *)&cfg_word;  // for GYRO X

    cfg_word.cs_vector = 0xF2;

    cfg_vector[i++] = *(uint32_t *)&cfg_word;  // for GYRO Y

    cfg_word.cs_vector = 0xF3;

    cfg_vector[i++] = *(uint32_t *)&cfg_word;  // for GYRO Z

    cfg_word.sclk_divider = SCLK_CLOCK_DIVIDER;
    cfg_word.tr_byte_num = 1;
    cfg_word.cs_vector = 0xF4;
    cfg_word.lsb_first = 0;
    cfg_word.clk_cpha = 1;
    cfg_word.clk_cpol = 1;
    cfg_word.clk_delay = 0;

    // for Acc XY ADIS16006
    cfg_vector[i++] = *(uint32_t *)&cfg_word;

    cfg_word.sclk_divider = SCLK_CLOCK_DIVIDER;
    cfg_word.tr_byte_num = 1;
    cfg_word.cs_vector = 0xF5;
    cfg_word.lsb_first = 0;
    cfg_word.clk_cpha = 1;
    cfg_word.clk_cpol = 1;
    cfg_word.clk_delay = 0;

    // for Acc XZ ADIS16006
    cfg_vector[i++] = *(uint32_t *)&cfg_word;

    cfg_word.sclk_divider = 10;
    cfg_word.tr_byte_num = 0;
    cfg_word.cs_vector = 0xF6;
    cfg_word.lsb_first = 0;
    cfg_word.clk_cpha = 1;
    cfg_word.clk_cpol = 1;
    cfg_word.clk_delay = 0;

    // for ADC7190
    cfg_vector[i++] = *(uint32_t *)&cfg_word;

    cfg_word.sclk_divider = SCLK_CLOCK_DIVIDER;
    cfg_word.tr_byte_num = 1;
    cfg_word.cs_vector = 0xF7;
    cfg_word.lsb_first = 0;
    cfg_word.clk_cpha = 1;
    cfg_word.clk_cpol = 1;
    cfg_word.clk_delay = 0;

    cfg_vector[i++] = *(uint32_t *)&cfg_word;  // for TEMP Acc XY ADIS16006

    cfg_word.sclk_divider = SCLK_CLOCK_DIVIDER;
    cfg_word.tr_byte_num = 1;
    cfg_word.cs_vector = 0xF8;
    cfg_word.lsb_first = 0;
    cfg_word.clk_cpha = 1;
    cfg_word.clk_cpol = 1;
    cfg_word.clk_delay = 0;

    cfg_vector[i++] = *(uint32_t *)&cfg_word;  // for TEMP Acc XY ADIS16006

    cfg_word.sclk_divider = 0;
    cfg_word.tr_byte_num = 2;
    cfg_word.lsb_first = 0;
    cfg_word.clk_cpha = 0;
    cfg_word.clk_cpol = 0;
    cfg_word.clk_delay = 0;
    cfg_word.cs_vector = 0xFF;

    for (j = i; j < SPIPLR_CFG_VECTOR_SIZE; j++) {
        cfg_vector[j] = *(uint32_t *)&cfg_word;
    }
}

void ctlst_sensors_imu_init_buffers(ctlst_sensors_imu_data_t *b) {
    // ADXRS450
    // Gyro X + температура
    b->fault_gx = ADXRS450_READ_OUTPUT(FCNAV_SPIPLR_DEV_ID_GYRO_X, 4);
    b->gyro_x =
        ADXRS450_REG_READ(FCNAV_SPIPLR_DEV_ID_GYRO_X, ADXRS450_REG_ADDRESS_TEM);
    b->temp_gx = ADXRS450_REG_READ(FCNAV_SPIPLR_DEV_ID_GYRO_X,
                                   ADXRS450_REG_ADDRESS_FAULT);

    // Gyro Y + температура
    b->fault_gy = ADXRS450_READ_OUTPUT(FCNAV_SPIPLR_DEV_ID_GYRO_Y, 6);
    b->gyro_y =
        ADXRS450_REG_READ(FCNAV_SPIPLR_DEV_ID_GYRO_Y, ADXRS450_REG_ADDRESS_TEM);
    b->temp_gy = ADXRS450_REG_READ(FCNAV_SPIPLR_DEV_ID_GYRO_Y,
                                   ADXRS450_REG_ADDRESS_FAULT);

    // Gyro Z + температура
    b->fault_gz = ADXRS450_READ_OUTPUT(FCNAV_SPIPLR_DEV_ID_GYRO_Z, 7);
    b->gyro_z =
        ADXRS450_REG_READ(FCNAV_SPIPLR_DEV_ID_GYRO_Z, ADXRS450_REG_ADDRESS_TEM);
    b->temp_gz = ADXRS450_REG_READ(FCNAV_SPIPLR_DEV_ID_GYRO_Z,
                                   ADXRS450_REG_ADDRESS_FAULT);

    // ADIS16006
    // Акселерометр XY
    b->accel_x = ADIS1600X_OUTPUT_READ_X(FCNAV_SPIPLR_DEV_ID_A_XY);
    b->accel_temp_xy = ADIS1600X_OUTPUT_READ_TEMP(FCNAV_SPIPLR_DEV_ID_TA_XY);
    b->accel_y = ADIS1600X_OUTPUT_READ_Y(FCNAV_SPIPLR_DEV_ID_A_XY);

    // Акселерометр ZY
    b->accel_y1 = ADIS1600X_OUTPUT_READ_X(FCNAV_SPIPLR_DEV_ID_A_YZ);
    b->accel_temp_zy = ADIS1600X_OUTPUT_READ_TEMP(FCNAV_SPIPLR_DEV_ID_TA_YZ);
    b->accel_z = ADIS1600X_OUTPUT_READ_Y(FCNAV_SPIPLR_DEV_ID_A_YZ);

    //	// AD7190
    b->ad7190_channel_seq[0] =
        SPI_PDC_TRANSMISSION(0, FCNAV_SPIPLR_DEV_ID_BADC,
                             AD7190_READ_REGISTER(AD7190_ADDR_REG_DATA));
    b->ad7190_channel_seq[1] =
        SPI_PDC_TRANSMISSION(0, FCNAV_SPIPLR_DEV_ID_BADC, 0xFF);
    b->ad7190_channel_seq[2] =
        SPI_PDC_TRANSMISSION(0, FCNAV_SPIPLR_DEV_ID_BADC, 0xFF);
    b->ad7190_channel_seq[3] =
        SPI_PDC_TRANSMISSION(0, FCNAV_SPIPLR_DEV_ID_BADC, 0xFF);
    b->ad7190_channel_seq[4] =
        SPI_PDC_TRANSMISSION(1, FCNAV_SPIPLR_DEV_ID_BADC, 0xFF);
}

static int ctlst_sensors_imu_spiplr_init(ctlst_sensors_imu_t *c) {
    spiplr_init(&c->spiplr_instance, SPIPLR_BASE_ADDR);
    uint32_t cfg_vector[SPIPLR_CFG_VECTOR_SIZE];
    ctlst_sensors_imu_init_cfg_array(cfg_vector);
    spiplr_cfg_vector_set(&c->spiplr_instance, cfg_vector,
                          SPIPLR_CFG_VECTOR_SIZE);
    spiplr_reset(&c->spiplr_instance);
    spiplr_poll_stop(&c->spiplr_instance);
    spiplr_poll_set_period(&c->spiplr_instance, c->cfg.period_us, 100000000);
    ctlst_sensors_imu_init_buffers(&c->data);
    int rv = spiplr_req_vector_set(&c->spiplr_instance, (uint32_t *)&c->data,
                                   sizeof(c->data) >> 2);
    return rv;
}

static void ctlst_sensors_imu_spiplr_irq_cnt(ctlst_sensors_imu_t *imu) {
    uint32_t cnt = spi_plr_get_irq_counter(&imu->spiplr_instance);

    if (imu->sync.irq_cnt_init) {
        imu->sync.irq_cnt++;
        if (cnt != imu->sync.irq_cnt) {
            int64_t delta = cnt - imu->sync.irq_cnt;
            if (delta < 0) {
                delta += UINT32_MAX;
            }
            error("Missed %d IRQs (irq_num %d)", delta, cnt);
            imu->sync.irq_cnt = cnt;
        }
    } else {
        imu->sync.irq_cnt = cnt;
        imu->sync.irq_cnt_init = -1;
    }

    imu->sync.irq_spiplr_cnt = cnt;
}

int adxrs450_parity_calc_32(uint32_t d) {
    int i;
    char p;
    p = (d >> 1) & 0x00000001;
    for (i = 2; i < 32; i++) {
        p ^= ((d >> i) & 0x01);
    }
    return (p == 1) ? 0 : 1;
}

int adxrs450_parity_check_32(uint32_t d) {
    return ((adxrs450_parity_calc_32(d) ^ (d & 0x00000001)) == 0) ? EOK
                                                                  : EBADMSG;
}
