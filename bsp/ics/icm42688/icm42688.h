#ifndef ICM42688_H
#define ICM42688_H

#include <errno.h>
#include <string.h>

#include "bit.h"
#include "const.h"
#include "core.h"
#include "log.h"
#include "os.h"
#include "periph.h"
#include "service.h"

#define ICM42688_FIFO_SIZE 2048
#define ICM42688_FIFO_SAMPLES 170

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
} icm42688_fifo_t;
#pragma pack(pop)

typedef struct {
    uint8_t CMD;
    uint8_t COUNTH;
    uint8_t COUNTL;
    icm42688_fifo_t buf[1];
} icm42688_fifo_buffer_t;

typedef struct {
    uint16_t bytes;
    uint16_t samples;
} icm42688_fifo_param_t;

typedef struct {
    double accel_x;
    double accel_y;
    double accel_z;
    double gyro_x;
    double gyro_y;
    double gyro_z;
    double imu_dt;
} icm42688_meas_t;

typedef struct {
    icm42688_meas_t meas[1];
    double temp;
} icm42688_meas_buffer_t;

typedef struct {
    double gyro_scale;
    double gyro_range;
    double accel_scale;
    double accel_range;
} icm42688_meas_param_t;

typedef struct {
    spi_t *spi;
    gpio_t *cs;
    exti_t *drdy;
} icm42688_interface_t;

typedef struct {
    SemaphoreHandle_t drdy_sem;
    SemaphoreHandle_t measrdy_sem;
    SemaphoreHandle_t mutex;
} icm42688_sync_t;

enum icm42688_state_t {
    ICM42688_RESET = 0,
    ICM42688_RESET_WAIT = 1,
    ICM42688_CONF = 2,
    ICM42688_FIFO_READ = 3,
    ICM42688_FAIL = 4
};

typedef struct {
    char name[MAX_NAME_LEN];
    icm42688_interface_t interface;
    service_t *service;
    icm42688_sync_t sync;
    icm42688_fifo_buffer_t fifo_buffer;
    icm42688_fifo_param_t fifo_param;
    icm42688_meas_buffer_t meas_buffer;
    icm42688_meas_t meas;
    icm42688_meas_param_t meas_param;
    enum icm42688_state_t state;
    uint8_t attempt;
} icm42688_t;

icm42688_t *icm42688_start(char *name, uint32_t period, uint32_t priority,
                          spi_t *spi, gpio_t *cs, exti_t *drdy);
void icm42688_get_meas_block(icm42688_t *dev, void *ptr);
void icm42688_get_meas_non_block(icm42688_t *dev, void *ptr);
void icm42688_stat(icm42688_t *dev);

#endif  // ICM42688_H
