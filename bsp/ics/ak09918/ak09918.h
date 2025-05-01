#ifndef AK09918_H
#define AK09918_H

#include <errno.h>
#include <string.h>

#include "bit.h"
#include "const.h"
#include "core.h"
#include "log.h"
#include "os.h"
#include "periph.h"
#include "service.h"

typedef struct {
    float mag_scale;
} ak09918_param_t;

typedef struct {
    float mag_x;
    float mag_y;
    float mag_z;
} ak09918_meas_t;

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
} ak09918_raw_t;

typedef struct {
    i2c_t *i2c;
    int isolated;  // Flag to indicate if magnetometer is isolated or body-fixed
    uint8_t rotation;  // Rotation configuration
} ak09918_interface_t;

typedef struct {
    SemaphoreHandle_t drdy_sem;
    SemaphoreHandle_t measrdy_sem;
    SemaphoreHandle_t mutex;
} ak09918_sync_t;

enum ak09918_state_t {
    AK09918_RESET,
    AK09918_RESET_WAIT,
    AK09918_CONF,
    AK09918_MEAS,
    AK09918_READ,
    AK09918_FAIL
};

typedef struct {
    char name[MAX_NAME_LEN];
    ak09918_interface_t interface;
    service_t *service;
    ak09918_sync_t sync;
    ak09918_raw_t raw;
    ak09918_meas_t meas;
    ak09918_param_t param;
    enum ak09918_state_t state;
    uint8_t attempt;
} ak09918_t;

ak09918_t *ak09918_start(char *name, uint32_t period, uint32_t priority,
                         i2c_t *i2c, int isolated, uint8_t rotation);
void ak09918_get_meas_block(ak09918_t *dev, void *ptr);
void ak09918_get_meas_non_block(ak09918_t *dev, void *ptr);
void ak09918_stat(ak09918_t *dev);

#endif  // AK09918_H 