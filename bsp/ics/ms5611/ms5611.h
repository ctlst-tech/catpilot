#ifndef MS5611_H
#define MS5611_H

#include <errno.h>
#include <string.h>

#include "bit.h"
#include "const.h"
#include "core.h"
#include "log.h"
#include "os.h"
#include "periph.h"
#include "service.h"

#define MS5611_PROM_SIZE 8

#pragma pack(push, 1)
typedef struct {
    uint16_t FACTORY;
    uint16_t C1;
    uint16_t C2;
    uint16_t C3;
    uint16_t C4;
    uint16_t C5;
    uint16_t C6;
    uint16_t SERIAL : 12;
    uint16_t CRC4 : 4;
} ms5611_prom_t;
#pragma pack(pop)

typedef struct {
    uint32_t D1;
    uint32_t D2;
} ms5611_raw_t;

typedef struct {
    double T;
    double P;
} ms5611_meas_t;

typedef enum {
    MS5611_READ_TEMP = 0,
    MS5611_READ_PRES_1 = 1,
    MS5611_READ_PRES_2 = 2,
    MS5611_READ_PRES_3 = 3,
    MS5611_READ_PRES_4 = 4,
} ms5611_meas_state_t;

typedef struct {
    spi_t *spi;
    gpio_t *cs;
} ms5611_interface_t;

typedef struct {
    SemaphoreHandle_t measrdy_sem;
} ms5611_sync_t;

enum ms5611_state_t {
    MS5611_RESET = 0,
    MS5611_READ_CALIB = 1,
    MS5611_READ_MEAS = 2,
    MS5611_FAIL = 3
};

typedef struct {
    char name[MAX_NAME_LEN];
    ms5611_interface_t interface;
    service_t *service;
    ms5611_sync_t sync;
    ms5611_prom_t calib;
    ms5611_raw_t raw;
    ms5611_meas_t meas;
    enum ms5611_state_t state;
    uint8_t attempt;
} ms5611_t;

ms5611_t *ms5611_start(const char *name, uint32_t period, uint32_t priority,
                       spi_t *spi, gpio_t *cs);
double ms5611_get_meas_block(ms5611_t *dev, void *ptr);
double ms5611_get_meas_non_block(ms5611_t *dev, void *ptr);
void ms5611_stat(ms5611_t *dev);

#endif  // MS5611_H
