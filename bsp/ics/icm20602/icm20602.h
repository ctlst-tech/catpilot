#ifndef ICM20602_H
#define ICM20602_H

#include <errno.h>
#include <string.h>

#include "bit.h"
#include "const.h"
#include "core.h"
#include "log.h"
#include "os.h"
#include "periph.h"
#include "service.h"

#define ICM20602_FIFO_SIZE 1008
#define ICM20602_FIFO_SAMPLES 72

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
} icm20602_fifo_t;
#pragma pack(pop)

typedef struct {
    uint8_t CMD;
    uint8_t COUNTH;
    uint8_t COUNTL;
    // icm20602_fifo_t buf[ICM20602_FIFO_SIZE / sizeof(icm20602_fifo_t) + 1];
    icm20602_fifo_t buf[1];
} icm20602_fifo_buffer_t;

typedef struct {
    uint16_t bytes;
    uint16_t samples;
} icm20602_fifo_param_t;

typedef struct {
    double accel_x;
    double accel_y;
    double accel_z;
    double gyro_x;
    double gyro_y;
    double gyro_z;
    double imu_dt;
} icm20602_meas_t;

typedef struct {
    // icm20602_meas_t meas[ICM20602_FIFO_SAMPLES];
    icm20602_meas_t meas[1];
    double temp;
} icm20602_meas_buffer_t;

typedef struct {
    double gyro_scale;
    double gyro_range;
    double accel_scale;
    double accel_range;
} icm20602_meas_param_t;

typedef struct {
    spi_t *spi;
    gpio_t *cs;
    exti_t *drdy;
} icm20602_interface_t;

typedef struct {
    SemaphoreHandle_t drdy_sem;
    SemaphoreHandle_t measrdy_sem;
    SemaphoreHandle_t mutex;
} icm20602_sync_t;

enum icm20602_state_t {
    ICM20602_RESET = 0,
    ICM20602_RESET_WAIT = 1,
    ICM20602_CONF = 2,
    ICM20602_FIFO_READ = 3,
    ICM20602_FAIL = 4
};

typedef struct {
    char name[MAX_NAME_LEN];
    icm20602_interface_t interface;
    service_t *service;
    icm20602_sync_t sync;
    icm20602_fifo_buffer_t fifo_buffer;
    icm20602_fifo_param_t fifo_param;
    icm20602_meas_buffer_t meas_buffer;
    icm20602_meas_t meas;
    icm20602_meas_param_t meas_param;
    enum icm20602_state_t state;
    uint8_t attempt;
} icm20602_t;

icm20602_t *icm20602_start(char *name, uint32_t period, uint32_t priority,
                           spi_t *spi, gpio_t *cs, exti_t *drdy);
void icm20602_get_meas_block(icm20602_t *dev, void *ptr);
void icm20602_get_meas_non_block(icm20602_t *dev, void *ptr);
void icm20602_stat(icm20602_t *dev);

#endif  // ICM20602_H
