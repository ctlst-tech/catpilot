#ifndef ICM20649_H
#define ICM20649_H

#include <errno.h>
#include <string.h>

#include "bit.h"
#include "const.h"
#include "core.h"
#include "log.h"
#include "os.h"
#include "periph.h"
#include "service.h"

#define ICM20649_FIFO_SIZE 4096
#define ICM20649_FIFO_SAMPLES 342

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
} icm20649_fifo_t;
#pragma pack(pop)

typedef struct {
    uint8_t CMD;
    uint8_t COUNTH;
    uint8_t COUNTL;
    // icm20649_fifo_t buf[ICM20649_FIFO_SIZE / sizeof(icm20649_fifo_t) + 1];
    icm20649_fifo_t buf[1];
} icm20649_fifo_buffer_t;

typedef struct {
    uint16_t bytes;
    uint16_t samples;
} icm20649_fifo_param_t;

typedef struct {
    double accel_x;
    double accel_y;
    double accel_z;
    double gyro_x;
    double gyro_y;
    double gyro_z;
    double imu_dt;
} icm20649_meas_t;

typedef struct {
    // icm20649_meas_t meas[ICM20649_FIFO_SAMPLES];
    icm20649_meas_t meas[1];
    double temp;
} icm20649_meas_buffer_t;

typedef struct {
    double gyro_scale;
    double gyro_range;
    double accel_scale;
    double accel_range;
} icm20649_meas_param_t;

typedef struct {
    spi_t *spi;
    gpio_t *cs;
    exti_t *drdy;
} icm20649_interface_t;

typedef struct {
    SemaphoreHandle_t drdy_sem;
    SemaphoreHandle_t measrdy_sem;
    SemaphoreHandle_t mutex;
} icm20649_sync_t;

enum icm20649_state_t {
    ICM20649_RESET = 0,
    ICM20649_RESET_WAIT = 1,
    ICM20649_CONF = 2,
    ICM20649_FIFO_READ = 3,
    ICM20649_FAIL = 4
};

typedef struct {
    char name[MAX_NAME_LEN];
    icm20649_interface_t interface;
    service_t *service;
    icm20649_sync_t sync;
    icm20649_fifo_buffer_t fifo_buffer;
    icm20649_fifo_param_t fifo_param;
    icm20649_meas_buffer_t meas_buffer;
    icm20649_meas_t meas;
    icm20649_meas_param_t meas_param;
    enum icm20649_state_t state;
    uint8_t attempt;
} icm20649_t;

icm20649_t *icm20649_start(char *name, uint32_t period, uint32_t priority,
                          spi_t *spi, gpio_t *cs, exti_t *drdy);
void icm20649_get_meas_block(icm20649_t *dev, void *ptr);
void icm20649_get_meas_non_block(icm20649_t *dev, void *ptr);
void icm20649_stat(icm20649_t *dev);

#endif  // ICM20649_H
