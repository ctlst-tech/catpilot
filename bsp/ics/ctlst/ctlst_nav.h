#ifndef CTLST_NAV_H
#define CTLST_NAV_H

#include "sens_spi.h"
#include "spiplr.h"

typedef struct {
    uint32_t parity_err;
    uint32_t rw_err;
    uint32_t st_invrdng;
    uint32_t st_cst_pll;
    uint32_t st_cst_q;
    uint32_t st_cst_nvm;
    uint32_t st_cst_por;
    uint32_t st_cst_pwr;
    uint32_t st_cst_cst;
} gyro_errors_t;

typedef struct {
    uint32_t spi_plr_poll_cnt;
    uint32_t sw_update_cnt;
    gyro_errors_t x;
    gyro_errors_t y;
    gyro_errors_t z;
} spi_sensors_stat_t;

int fcnav_init(uint32_t req_period_us);
void fcnav_irq_en();
void fcnav_irq_dis();
void fcnav_irq_cnt_mon();
void fcnav_poll_start();
void fcnav_poll_stop();
uint32_t fcnav_read_status();
void fcnav_reset_status();
int fcnav_spi_trans(uint32_t *inb, int n, uint32_t *outb);

uint32_t ssd_fcnav_startup_diag();
void fcnav_get_spi_sensors_stat(spi_sensors_stat_t *adc_data);
int ssd_fcnav_raw_to_adc_data(void *d, sens_onboard_adc_data_t *adc_data);

#endif  // CTLST_NAV_H
