#ifndef ICM20948_H
#define ICM20948_H

#include <errno.h>
#include <string.h>

#include "bit.h"
#include "const.h"
#include "core.h"
#include "log.h"
#include "os.h"
#include "periph.h"

#define ICM20948_FIFO_SIZE 4096
#define ICM20948_FIFO_SAMPLES 342

#pragma pack(push, 1)
typedef struct {
    uint8_t ACCEL_XOUT_H;
    uint8_t ACCEL_XOUT_L;
    uint8_t ACCEL_YOUT_H;
    uint8_t ACCEL_YOUT_L;
    uint8_t ACCEL_ZOUT_H;
    uint8_t ACCEL_ZOUT_L;
    uint8_t GYRO_XOUT_H;
    uint8_t GYRO_XOUT_L;
    uint8_t GYRO_YOUT_H;
    uint8_t GYRO_YOUT_L;
    uint8_t GYRO_ZOUT_H;
    uint8_t GYRO_ZOUT_L;
} icm20948_fifo_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint8_t ST1;
    uint8_t HXL;
    uint8_t HXH;
    uint8_t HYL;
    uint8_t HYH;
    uint8_t HZL;
    uint8_t HZH;
    uint8_t TMPS;
    uint8_t ST2;
} icm20948_mag_t;
#pragma pack(pop)

typedef struct {
    uint8_t CMD;
    uint8_t COUNTH;
    uint8_t COUNTL;
    icm20948_fifo_t buf[ICM20948_FIFO_SIZE / sizeof(icm20948_fifo_t) + 1];
} icm20948_fifo_buffer_t;

typedef struct {
    uint16_t bytes;
    uint16_t samples;
} icm20948_fifo_param_t;

typedef struct {
    double accel_x;
    double accel_y;
    double accel_z;
    double gyro_x;
    double gyro_y;
    double gyro_z;
    double imu_dt;
} icm20948_meas_t;

typedef struct {
    icm20948_meas_t meas[ICM20948_FIFO_SAMPLES];
    double temp;
} icm20948_meas_buffer_t;

typedef struct {
    double gyro_scale;
    double gyro_range;
    double accel_scale;
    double accel_range;
} icm20948_meas_param_t;

typedef struct {
    spi_t *spi;
    gpio_t *cs;
    exti_t *drdy;
} icm20948_interface_t;

typedef struct {
    SemaphoreHandle_t drdy_sem;
    SemaphoreHandle_t measrdy_sem;
} icm20948_sync_t;

typedef struct {
    uint32_t period;
    uint32_t priority;
} icm20948_thread_t;

enum icm20948_state_t {
    ICM20948_RESET = 0,
    ICM20948_RESET_WAIT = 1,
    ICM20948_CONF = 2,
    ICM20948_FIFO_READ = 3,
    ICM20948_FAIL = 4
};

typedef struct {
    char name[32];
    icm20948_interface_t interface;
    icm20948_sync_t sync;
    icm20948_fifo_buffer_t fifo_buffer;
    icm20948_fifo_param_t fifo_param;
    icm20948_meas_buffer_t meas_buffer;
    icm20948_meas_t meas;
    icm20948_meas_param_t meas_param;
    icm20948_thread_t os;
    enum icm20948_state_t state;
    uint8_t attempt;
    int enable_mag;
} icm20948_t;

int icm20948_start(spi_t *spi, gpio_t *cs, exti_t *drdy, uint32_t period,
                   uint32_t thread_priority, int enable_mag);
void icm20948_get_meas_block(icm20948_t *dev, void *ptr);
void icm20948_get_meas_non_block(icm20948_t *dev, void *ptr);

#endif  // ICM20948_H
