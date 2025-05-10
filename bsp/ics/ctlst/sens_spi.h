#ifndef SENS_SPI_H_
#define SENS_SPI_H_

#include <stdint.h>

typedef struct _sens_onboard_adc_data {
    int32_t Ax;
    int32_t Ay;
    int32_t Az;

    int32_t TAx;
    int32_t TAy;
    int32_t TAz;

    int32_t Gx;
    int32_t Gy;
    int32_t Gz;

    int32_t TGx;
    int32_t TGy;
    int32_t TGz;

    int32_t Pstat;
    int32_t Pdiff;

    int32_t Pdyn;
    int32_t PadcT;

} sens_onboard_adc_data_t;

typedef struct _sens_onboard_data {
    float Ax;
    float Ay;
    float Az;

    float TAx;
    float TAy;
    float TAz;

    float wx;
    float wy;
    float wz;

    float Twx;
    float Twy;
    float Twz;

    float Pstat;
    float Pdiff;
    float Pdyn;
    float PadcT;

} sens_onboard_data_t;

#define DIRECTION_FORWARD 1
#define DIRECTION_REVERSED -1

#define SENS_SPI_DIAG_FAILURE_FLAG_GX (1 << 0)
#define SENS_SPI_DIAG_FAILURE_FLAG_GY (1 << 1)
#define SENS_SPI_DIAG_FAILURE_FLAG_GZ (1 << 2)

#define SENS_SPI_DIAG_GYROSCOPES_MASK                                \
    (SENS_SPI_DIAG_FAILURE_FLAG_GX | SENS_SPI_DIAG_FAILURE_FLAG_GY | \
     SENS_SPI_DIAG_FAILURE_FLAG_GZ)

#define SENS_SPI_DIAG_FAILURE_FLAG_AX (1 << 3)
#define SENS_SPI_DIAG_FAILURE_FLAG_AY (1 << 4)
#define SENS_SPI_DIAG_FAILURE_FLAG_AZ (1 << 5)

#define SENS_SPI_DIAG_ACCELEROMETERS_MASK                            \
    (SENS_SPI_DIAG_FAILURE_FLAG_AX | SENS_SPI_DIAG_FAILURE_FLAG_AY | \
     SENS_SPI_DIAG_FAILURE_FLAG_AZ)

#define SENS_SPI_DIAG_INERTIAL_MASK \
    (SENS_SPI_DIAG_GYROSCOPES_MASK | SENS_SPI_DIAG_ACCELEROMETERS_MASK)

#define SENS_SPI_DIAG_FAILURE_FLAG_ADS_ADC (1 << 6)
#define SENS_SPI_DIAG_FAILURE_FLAG_ADS_BAR (1 << 7)

typedef struct _sens_spi_device_config {
    int (*raw_to_adc_data)(void *raw, sens_onboard_adc_data_t *adc);
    int (*pdc_tx_buff_init)(void *b);

    uint32_t (*startup_diag_procedure)(void);

    uint32_t sizeof_tx_buffer;
    uint32_t pdc_tx_num;

    uint32_t request_period_us;

    float gyro_scale;
    float gyro_offset;

    float gyro_temp_scale;
    float gyro_temp_offset;

    float accel_offset;
    float accel_scale;

    float accel_temp_scale;
    float accel_temp_offset;

    float pabs_scale;
    float pabs_offset;

    float pdiff_scale;
    float pdiff_offset;

    int dir_gx;
    int dir_gy;
    int dir_gz;

    int dir_ax;
    int dir_ay;
    int dir_az;

    struct _chip_select_opts {
        uint32_t settings;
        uint32_t baudrate;
    } cs[4];

} sens_spi_device_config_t;

#define SENS_SPI_GET_DATA_IMU_ANG_RATE (1 << 0)
#define SENS_SPI_GET_DATA_IMU_ACCEL (1 << 1)
#define SENS_SPI_GET_DATA_BAR (1 << 2)
#define SENS_SPI_GET_DATA_IMU \
    (SENS_SPI_GET_DATA_IMU_ANG_RATE | SENS_SPI_GET_DATA_IMU_ACCEL)
#define SENS_SPI_GET_DATA_ALL                                       \
    (SENS_SPI_GET_DATA_IMU_ANG_RATE | SENS_SPI_GET_DATA_IMU_ACCEL | \
     SENS_SPI_GET_DATA_BAR)

#endif /*SENS_SPI_H_*/
