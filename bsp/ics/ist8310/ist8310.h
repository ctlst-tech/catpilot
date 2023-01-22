#ifndef IST8310_H
#define IST8310_H

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
} ist8310_param_t;

typedef struct {
    float mag_x;
    float mag_y;
    float mag_z;
} ist8310_meas_t;

typedef struct {
    uint8_t STAT1;
    uint8_t DATAXL;
    uint8_t DATAXH;
    uint8_t DATAYL;
    uint8_t DATAYH;
    uint8_t DATAZL;
    uint8_t DATAZH;
} ist8310_raw_t;

typedef struct {
    i2c_t *i2c;
} ist8310_interface_t;

typedef struct {
    SemaphoreHandle_t drdy_sem;
    SemaphoreHandle_t measrdy_sem;
    SemaphoreHandle_t mutex;
} ist8310_sync_t;

enum ist8310_state_t {
    IST8310_RESET,
    IST8310_RESET_WAIT,
    IST8310_CONF,
    IST8310_MEAS,
    IST8310_READ,
    IST8310_FAIL
};

typedef struct {
    char name[MAX_NAME_LEN];
    ist8310_interface_t interface;
    service_t *service;
    ist8310_sync_t sync;
    ist8310_raw_t raw;
    ist8310_meas_t meas;
    ist8310_param_t param;
    enum ist8310_state_t state;
    uint8_t attempt;
} ist8310_t;

ist8310_t *ist8310_start(char *name, uint32_t period, uint32_t priority,
                         i2c_t *i2c);
void ist8310_get_meas_block(ist8310_t *dev, void *ptr);
void ist8310_get_meas_non_block(ist8310_t *dev, void *ptr);
void ist8310_stat(ist8310_t *dev);

#endif  // IST8310_H
