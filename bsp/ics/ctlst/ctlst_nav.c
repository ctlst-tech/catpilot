#include "ctlst_nav.h"

#include <atomic.h>
#include <errno.h>
#include <f/fsyslog.h>
#include <pthread.h>
#include <unistd.h>

#include "ad7190.h"
#include "adis1600X.h"
#include "adxrs450.h"
#include "ctlst_baro.h"
#include "ctlst_periph.h"
#include "devices_common.h"
#include "sens_spi.h"

#define FCNAV_SPIPLR_DEV_ID_GYRO_X 2
#define FCNAV_SPIPLR_DEV_ID_GYRO_Y 3
#define FCNAV_SPIPLR_DEV_ID_GYRO_Z 4
#define FCNAV_SPIPLR_DEV_ID_A_XY 5
#define FCNAV_SPIPLR_DEV_ID_A_YZ 6
#define FCNAV_SPIPLR_DEV_ID_BADC 7
#define FCNAV_SPIPLR_DEV_ID_TA_XY 8
#define FCNAV_SPIPLR_DEV_ID_TA_YZ 9

typedef struct __attribute__((packed)) {
    // ADXRS450
    uint64_t fault_gx;
    uint64_t gyro_x;
    uint64_t temp_gx;

    uint64_t fault_gy;
    uint64_t gyro_y;
    uint64_t temp_gy;

    uint64_t fault_gz;
    uint64_t gyro_z;
    uint64_t temp_gz;

    // ADIS16006
    uint32_t accel_y1;
    uint32_t accel_temp_zy;
    uint32_t accel_z;

    uint32_t accel_x;
    uint32_t accel_temp_xy;
    uint32_t accel_y;

    // AD7190
    uint32_t ad7190_channel_seq[5];
} p_data_t;

static spiplr_instance_t spiplr_i;

void init_req_buffer(p_data_t *b) {
    // ADXRS450
    b->fault_gx = ADXRS450_READ_OUTPUT(FCNAV_SPIPLR_DEV_ID_GYRO_X, 4);
    b->gyro_x =
        ADXRS450_REG_READ(FCNAV_SPIPLR_DEV_ID_GYRO_X, ADXRS450_REG_ADDRESS_TEM);
    b->temp_gx = ADXRS450_REG_READ(FCNAV_SPIPLR_DEV_ID_GYRO_X,
                                   ADXRS450_REG_ADDRESS_FAULT);

    b->fault_gy = ADXRS450_READ_OUTPUT(FCNAV_SPIPLR_DEV_ID_GYRO_Y, 6);
    b->gyro_y =
        ADXRS450_REG_READ(FCNAV_SPIPLR_DEV_ID_GYRO_Y, ADXRS450_REG_ADDRESS_TEM);
    b->temp_gy = ADXRS450_REG_READ(FCNAV_SPIPLR_DEV_ID_GYRO_Y,
                                   ADXRS450_REG_ADDRESS_FAULT);

    b->fault_gz = ADXRS450_READ_OUTPUT(FCNAV_SPIPLR_DEV_ID_GYRO_Z, 7);
    b->gyro_z =
        ADXRS450_REG_READ(FCNAV_SPIPLR_DEV_ID_GYRO_Z, ADXRS450_REG_ADDRESS_TEM);
    b->temp_gz = ADXRS450_REG_READ(FCNAV_SPIPLR_DEV_ID_GYRO_Z,
                                   ADXRS450_REG_ADDRESS_FAULT);

    // ADIS16006
    b->accel_x = ADIS1600X_OUTPUT_READ_X(FCNAV_SPIPLR_DEV_ID_A_XY);
    b->accel_temp_xy = ADIS1600X_OUTPUT_READ_TEMP(FCNAV_SPIPLR_DEV_ID_TA_XY);
    b->accel_y = ADIS1600X_OUTPUT_READ_Y(FCNAV_SPIPLR_DEV_ID_A_XY);

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

static p_data_t req_buffer;

static void init_cfg_array(uint32_t *cfg_vector) {
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

#define SCLK_CLOCK_DIVIDER_ADIS160006 25

    cfg_word.sclk_divider = SCLK_CLOCK_DIVIDER_ADIS160006;
    cfg_word.tr_byte_num = 1;
    cfg_word.cs_vector = 0xF4;
    cfg_word.lsb_first = 0;
    cfg_word.clk_cpha = 1;
    cfg_word.clk_cpol = 1;
    cfg_word.clk_delay = 0;

    cfg_vector[i++] = *(uint32_t *)&cfg_word;  // for Acc XY ADIS16006

    cfg_word.sclk_divider = SCLK_CLOCK_DIVIDER_ADIS160006;
    cfg_word.tr_byte_num = 1;
    cfg_word.cs_vector = 0xF5;
    cfg_word.lsb_first = 0;
    cfg_word.clk_cpha = 1;
    cfg_word.clk_cpol = 1;
    cfg_word.clk_delay = 0;

    cfg_vector[i++] = *(uint32_t *)&cfg_word;  // for Acc XZ ADIS16006

    cfg_word.sclk_divider = 10;
    cfg_word.tr_byte_num = 0;
    cfg_word.cs_vector = 0xF6;
    cfg_word.lsb_first = 0;
    cfg_word.clk_cpha = 1;
    cfg_word.clk_cpol = 1;
    cfg_word.clk_delay = 0;

    cfg_vector[i++] = *(uint32_t *)&cfg_word;  // for ADC7190

    cfg_word.sclk_divider = SCLK_CLOCK_DIVIDER_ADIS160006;
    cfg_word.tr_byte_num = 1;
    cfg_word.cs_vector = 0xF7;
    cfg_word.lsb_first = 0;
    cfg_word.clk_cpha = 1;
    cfg_word.clk_cpol = 1;
    cfg_word.clk_delay = 0;

    cfg_vector[i++] = *(uint32_t *)&cfg_word;  // for TEMP Acc XY ADIS16006

    cfg_word.sclk_divider = SCLK_CLOCK_DIVIDER_ADIS160006;
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

int fcnav_init(uint32_t req_period_us) {
    spiplr_init(&spiplr_i, SPIPLR_BASE_ADDR);

    uint32_t cfg_vector[SPIPLR_CFG_VECTOR_SIZE];

    init_cfg_array(cfg_vector);

    spiplr_cfg_vector_set(&spiplr_i, cfg_vector, SPIPLR_CFG_VECTOR_SIZE);

    spiplr_reset(&spiplr_i);

    spiplr_poll_stop(&spiplr_i);

    spiplr_poll_set_period(&spiplr_i, req_period_us, 100000000);

    init_req_buffer(&req_buffer);

    int rv = spiplr_req_vector_set(&spiplr_i, (uint32_t *)&req_buffer,
                                   sizeof(req_buffer) >> 2);
    return rv;
}

void fcnav_irq_en() {
    spiplr_irq_poll_rdy_enable(&spiplr_i);
}

void fcnav_irq_dis() {
    spiplr_irq_poll_rdy_enable(&spiplr_i);
}

static uint32_t irq_counter = 0;
volatile static uint32_t irq_spiplr_cnt;
static int irq_counter_inited = 0;

void fcnav_irq_cnt_mon() {
    uint32_t cnt = spi_plr_get_irq_counter(&spiplr_i);

    if (irq_counter_inited) {
        irq_counter++;
        if (cnt != irq_counter) {
            int64_t delta = cnt - irq_counter;
            if (delta < 0) {
                delta += UINT32_MAX;
            }
            error("Missed %d IRQs (irq_num %d)", delta, cnt);
            irq_counter = cnt;
        }
    } else {
        irq_counter = cnt;
        irq_counter_inited = -1;
    }

    irq_spiplr_cnt = cnt;
}

void fcnav_poll_start() {
    spiplr_poll_start(&spiplr_i);
}

void fcnav_poll_stop() {
    spiplr_poll_stop(&spiplr_i);
}

uint32_t fcnav_read_status() {
    uint32_t st;
    spiplr_get_status(&spiplr_i, &st);

    return st;
}

void fcnav_reset_status() {
    spiplr_reset_status(&spiplr_i);
}

#define AD7190_CHANNEL_DIFF 0
#define AD7190_CHANNEL_TEMPR 2
#define AD7190_CHANNEL_ABS 4
#define AD7190_CHANNEL_DYN 6

#define AD7190_CHANNEL_BIAS 0x800000

int fc_dxrs450_parity_check_32(uint32_t d);

static uint32_t fcnav_data_update_counter;

#ifdef GYRO_DEBUG
#define report_event(txt, ...)                                            \
    error("Rdng %08d (val %08X) | %-10s | " txt, update_counter, reading, \
          dev_name, ##__VA_ARGS__)
#else
#define report_event(txt, ...)
#endif

typedef enum {
    gyro_ok = 0,
    gyro_rw_err = 1,
    gyro_st_inval_rdngr = 2,
    gyro_st_flags = 3
} gyro_test_rv_t;

static gyro_test_rv_t get_gyro_errs_and_report(uint32_t reading,
                                               gyro_errors_t *stat,
                                               const char *dev_name) {
    uint32_t errs = reading & (0xFF & (~0x03));

    gyro_test_rv_t rv = gyro_ok;

#define SPI (1 << 18)
#define RE (1 << 17)
#define DU (1 << 16)

    if (fc_dxrs450_parity_check_32(reading) != EOK) {
        report_event("Gyro %s parity error");
        stat->parity_err++;
        rv = gyro_rw_err;
    }

    switch (reading >> 29) {
        case 0:
            report_event("RW error: %s %s %s", reading & SPI ? "SPI" : "",
                         reading & RE ? "RE" : "", reading & DU ? "DU" : "");
            stat->rw_err++;
            rv = gyro_rw_err;
            break;

        // reg read response
        case 2:
            break;

        // reg write reposnse
        case 1:
            report_event("Unexpected write response");
            rv = 1;
            break;

        // Gyros (according to SEQ number in request)
        case 4:
        case 6:
        case 7:
            switch ((reading >> 26) & 0x03) {
                case 0:
                    report_event("ST bits: invalid data for sensor resp");
                    stat->st_invrdng++;
                    rv = gyro_st_inval_rdngr;
                    break;

                case 2:
                    report_event("ST bits: sensor self test data");
                    rv = gyro_st_inval_rdngr;
                    break;

                default:
                case 1:
                    break;
            }

            if (errs) {
                if (errs & ADXRS450_OUTPUT_CST_PLL) {
                    report_event("Selftest bit PLL is set");
                    stat->st_cst_pll++;
                }
                if (errs & ADXRS450_OUTPUT_CST_Q) {
                    report_event("Selftest bit Q is set");
                    stat->st_cst_q++;
                }
                if (errs & ADXRS450_OUTPUT_CST_NVM) {
                    report_event("Selftest bit NVM is set");
                    stat->st_cst_nvm++;
                }
                if (errs & ADXRS450_OUTPUT_CST_POR) {
                    report_event("Selftest bit POR is set");
                    stat->st_cst_por++;
                }
                if (errs & ADXRS450_OUTPUT_CST_PWR) {
                    report_event("Selftest bit PWR is set");
                    stat->st_cst_pwr++;
                }
                if (errs & ADXRS450_OUTPUT_CST_CST) {
                    report_event("Selftest bit CST is set");
                    stat->st_cst_cst++;
                }
                rv = gyro_st_flags;
            }
            break;

        default:
            report_event("Gyro %s unrecognized response");
            break;
    }

    return rv;
}

pthread_mutex_t spi_sensors_stat_mutex = PTHREAD_MUTEX_INITIALIZER;
static spi_sensors_stat_t spi_sensors_stat;

void fcnav_get_spi_sensors_stat(spi_sensors_stat_t *adc_data) {
    pthread_mutex_lock(&spi_sensors_stat_mutex);
    *adc_data = spi_sensors_stat;
    pthread_mutex_unlock(&spi_sensors_stat_mutex);
}

int fcnav_get_data(sens_onboard_adc_data_t *adc_data,
                   spi_sensors_stat_t *stat) {
    p_data_t d;
    p_data_t *p = &d;

    fcnav_data_update_counter++;

    uint32_t ad7190_data;

    int resp_size = sizeof(d);
    spiplr_resp_vector_get(&spiplr_i, (uint32_t *)&d, &resp_size);

    adc_data->Ax = SPI_PDC_EXTRACT_STRUCT_WORD(p, accel_x);
    adc_data->Ay = SPI_PDC_EXTRACT_STRUCT_WORD(p, accel_y);
    adc_data->Az = SPI_PDC_EXTRACT_STRUCT_WORD(p, accel_z);

    adc_data->TAx =
        TWOS_CONVERT10(SPI_PDC_EXTRACT_STRUCT_WORD(p, accel_temp_xy) >> 5);
    adc_data->TAy = adc_data->TAx;
    adc_data->TAz =
        TWOS_CONVERT10(SPI_PDC_EXTRACT_STRUCT_WORD(p, accel_temp_zy) >> 5);

    uint32_t gyro_x_reading = SPI_PDC_EXTRACT_STRUCT_DWORD(p, gyro_x);
    uint32_t gyro_y_reading = SPI_PDC_EXTRACT_STRUCT_DWORD(p, gyro_y);
    uint32_t gyro_z_reading = SPI_PDC_EXTRACT_STRUCT_DWORD(p, gyro_z);

    adc_data->Gx = TWOS_CONVERT16((gyro_x_reading >> 10) & (0xFFFF));
    adc_data->Gy = TWOS_CONVERT16((gyro_y_reading >> 10) & (0xFFFF));
    adc_data->Gz = TWOS_CONVERT16((gyro_z_reading >> 10) & (0xFFFF));

    get_gyro_errs_and_report(gyro_x_reading, &stat->x, "X rate");
    get_gyro_errs_and_report(gyro_y_reading, &stat->y, "Y rate");
    get_gyro_errs_and_report(gyro_z_reading, &stat->z, "Z rate");

    uint32_t gyro_x_t_reading = SPI_PDC_EXTRACT_STRUCT_DWORD(p, temp_gx);
    uint32_t gyro_y_t_reading = SPI_PDC_EXTRACT_STRUCT_DWORD(p, temp_gy);
    uint32_t gyro_z_t_reading = SPI_PDC_EXTRACT_STRUCT_DWORD(p, temp_gz);

    adc_data->TGx = TWOS_CONVERT10((gyro_x_t_reading >> 11) & (0x3FF));
    adc_data->TGy = TWOS_CONVERT10((gyro_y_t_reading >> 11) & (0x3FF));
    adc_data->TGz = TWOS_CONVERT10((gyro_z_t_reading >> 11) & (0x3FF));

    get_gyro_errs_and_report(gyro_x_t_reading, &stat->x, "X tempr");
    get_gyro_errs_and_report(gyro_y_t_reading, &stat->y, "Y tempr");
    get_gyro_errs_and_report(gyro_z_t_reading, &stat->z, "Z tempr");

    uint32_t gyro_x_f_reading = SPI_PDC_EXTRACT_STRUCT_DWORD(p, fault_gx);
    uint32_t gyro_y_f_reading = SPI_PDC_EXTRACT_STRUCT_DWORD(p, fault_gy);
    uint32_t gyro_z_f_reading = SPI_PDC_EXTRACT_STRUCT_DWORD(p, fault_gz);

    get_gyro_errs_and_report(gyro_x_f_reading, &stat->x, "X fault");
    get_gyro_errs_and_report(gyro_y_f_reading, &stat->y, "Y fault");
    get_gyro_errs_and_report(gyro_z_f_reading, &stat->z, "Z fault");

    ad7190_data =
        (SPI_PDC_EXTRACT_STRUCT_BYTE(p, ad7190_channel_seq[1]) << 24) |
        (SPI_PDC_EXTRACT_STRUCT_BYTE(p, ad7190_channel_seq[2]) << 16) |
        (SPI_PDC_EXTRACT_STRUCT_BYTE(p, ad7190_channel_seq[3]) << 8) |
        SPI_PDC_EXTRACT_STRUCT_BYTE(p, ad7190_channel_seq[4]);

    switch (AD7190_DATA_EXTRACT_CHNL(AD7190_MASK_LSB(ad7190_data))) {
        case AD7190_CHANNEL_DIFF:
            adc_data->Pdiff =
                (AD7190_CHANNEL_BIAS - AD7190_DATA_EXTRACT(ad7190_data));
            break;

        case AD7190_CHANNEL_ABS:
            adc_data->Pstat =
                AD7190_DATA_EXTRACT(ad7190_data) - AD7190_CHANNEL_BIAS;
            break;

        case AD7190_CHANNEL_DYN:
            adc_data->Pdyn =
                AD7190_DATA_EXTRACT(ad7190_data) - AD7190_CHANNEL_BIAS;
            break;

        case AD7190_CHANNEL_TEMPR:
            adc_data->PadcT = AD7190_DATA_EXTRACT(ad7190_data);
            break;

        default:
            break;
    }

    return EOK;
}

int ssd_fcnav_buff_init(void *d) {
    return EOK;
}

int ssd_fcnav_raw_to_adc_data(void *d, sens_onboard_adc_data_t *adc_data) {
    pthread_mutex_lock(&spi_sensors_stat_mutex);
    spi_sensors_stat.spi_plr_poll_cnt = irq_spiplr_cnt;
    spi_sensors_stat.sw_update_cnt = fcnav_data_update_counter;
    fcnav_get_data(adc_data, &spi_sensors_stat);
    pthread_mutex_unlock(&spi_sensors_stat_mutex);
    return EOK;
}

int fc_dxrs450_diag(int cs, int dcs, uint32_t *sn, uint16_t *fault_reg,
                    uint32_t *diag_res);

static void dry_read() {
    uint32_t sn = 0;
    uint32_t diag_res = 0;
    uint16_t fault_reg = 0;
    fc_dxrs450_diag(FCNAV_SPIPLR_DEV_ID_GYRO_X, 0, &sn, &fault_reg, &diag_res);
}

uint32_t ssd_fcnav_startup_diag() {
    uint32_t flags = 0;

    dry_read();

    flags |= (fc_dxrs450_diag_and_report("Gyro X", FCNAV_SPIPLR_DEV_ID_GYRO_X,
                                         0) == EOK)
                 ? 0
                 : SENS_SPI_DIAG_FAILURE_FLAG_GX;
    flags |= (fc_dxrs450_diag_and_report("Gyro Y", FCNAV_SPIPLR_DEV_ID_GYRO_Y,
                                         0) == EOK)
                 ? 0
                 : SENS_SPI_DIAG_FAILURE_FLAG_GY;
    flags |= (fc_dxrs450_diag_and_report("Gyro Z", FCNAV_SPIPLR_DEV_ID_GYRO_Z,
                                         0) == EOK)
                 ? 0
                 : SENS_SPI_DIAG_FAILURE_FLAG_GZ;

    flags |= (fc_d7190_start_and_report(FCNAV_SPIPLR_DEV_ID_BADC, 0, 1) == EOK)
                 ? 0
                 : SENS_SPI_DIAG_FAILURE_FLAG_ADS_ADC;

    return flags;
}

int fcnav_spi_trans(uint32_t *inb, int n, uint32_t *outb) {
    return spi_plr_custom_trans(&spiplr_i, inb, n, outb);
}

sens_spi_device_config_t cfg = {

    .pdc_tx_buff_init = ssd_fcnav_buff_init,
    .raw_to_adc_data = ssd_fcnav_raw_to_adc_data,

    .startup_diag_procedure = ssd_fcnav_startup_diag,

    .request_period_us = 2000,

    .gyro_scale = ADXRS450_SCALE,

    .gyro_temp_scale = ADXRS450_TEMP_SCALE,
    .gyro_temp_offset = ADXRS450_TEMP_OFFSET,

    .accel_offset = ADIS1600X_OFFSET,
    .accel_scale = ADIS16006_SCALE,

    .accel_temp_scale = ADIS1600X_TEMP_SCALE,
    .accel_temp_offset = ADIS1600X_TEMP_OFFSET,

    .pabs_scale = PABS_SCALE,
    .pabs_offset = PABS_OFFSET,

    .pdiff_scale = PDIFF_SCALE,
    .pdiff_offset = PDIFF_OFFSET,

    .dir_gx = DIRECTION_FORWARD,
    .dir_gy = DIRECTION_REVERSED,
    .dir_gz = DIRECTION_FORWARD,

    .dir_ax = DIRECTION_FORWARD,
    .dir_ay = DIRECTION_FORWARD,
    .dir_az = DIRECTION_REVERSED,

    .cs = {{.baudrate = 1000000},
           {.baudrate = 1000000},
           {.baudrate = 1000000},
           {.baudrate = 1000000}}};
