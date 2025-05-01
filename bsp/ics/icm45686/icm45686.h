#ifndef ICM45686_H
#define ICM45686_H

#include <errno.h>
#include <string.h>

#include "bit.h"
#include "const.h"
#include "core.h"
#include "log.h"
#include "os.h"
#include "periph.h"
#include "service.h"

#define ICM45686_FIFO_SIZE 2048
#define ICM45686_FIFO_SAMPLES 342

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
    uint8_t TEMP_DATA1_H;
    uint8_t TEMP_DATA1_L;
    uint8_t TMST_FSYNCH;
    uint8_t TMST_FSYNCL;
} icm45686_fifo_t;
#pragma pack(pop)

typedef struct {
    uint8_t CMD;
    uint8_t COUNTH;
    uint8_t COUNTL;
    icm45686_fifo_t buf[1];
} icm45686_fifo_buffer_t;

typedef struct {
    uint16_t bytes;
    uint16_t samples;
} icm45686_fifo_param_t;

typedef struct {
    double accel_x;
    double accel_y;
    double accel_z;
    double gyro_x;
    double gyro_y;
    double gyro_z;
    double imu_dt;
} icm45686_meas_t;

typedef struct {
    icm45686_meas_t meas[1];
    double temp;
} icm45686_meas_buffer_t;

typedef struct {
    double gyro_scale;
    double gyro_range;
    double accel_scale;
    double accel_range;
} icm45686_meas_param_t;

typedef struct {
    spi_t *spi;
    gpio_t *cs;
    exti_t *drdy;
} icm45686_interface_t;

typedef struct {
    SemaphoreHandle_t measrdy_sem;
    SemaphoreHandle_t mutex;
} icm45686_sync_t;

// Device states
enum icm45686_state_t {
    ICM45686_RESET = 0,
    ICM45686_RESET_WAIT = 1,
    ICM45686_CONF = 2,
    ICM45686_FIFO_READ = 3,
    ICM45686_FAIL = 4
};

typedef struct {
    char name[MAX_NAME_LEN];
    icm45686_interface_t interface;
    service_t *service;
    icm45686_sync_t sync;
    icm45686_fifo_buffer_t fifo_buffer;
    icm45686_fifo_param_t fifo_param;
    icm45686_meas_buffer_t meas_buffer;
    icm45686_meas_t meas;
    icm45686_meas_param_t meas_param;
    enum icm45686_state_t state;
    uint8_t attempt;
} icm45686_t;

icm45686_t *icm45686_start(char *name, uint32_t period, uint32_t priority, spi_t *spi, gpio_t *cs);
void icm45686_get_meas_block(icm45686_t *dev, void *ptr);
void icm45686_get_meas_non_block(icm45686_t *dev, void *ptr);
void icm45686_stat(icm45686_t *dev);

#endif  // ICM45686_H 