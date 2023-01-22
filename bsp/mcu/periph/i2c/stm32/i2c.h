#ifndef I2C_H
#define I2C_H

#include <errno.h>

#include "core.h"
#include "dma.h"
#include "gpio.h"
#include "hal.h"

enum i2c_state_t {
    I2C_FREE = 0,
    I2C_TRANSMIT = 1,
    I2C_RECEIVE = 2,
};

typedef struct {
    IRQn_Type ev_id;
    IRQn_Type er_id;
    SemaphoreHandle_t sem;
    SemaphoreHandle_t mutex;
    SemaphoreHandle_t cs_mutex;
    enum i2c_state_t state;
} i2c_private_t;

typedef struct {
    I2C_HandleTypeDef init;
    gpio_t *scl;
    gpio_t *sda;
    dma_t dma_tx;
    dma_t dma_rx;
    int timeout;
    int irq_priority;
    i2c_private_t p;
} i2c_t;

int i2c_init(i2c_t *cfg);
int i2c_transmit(i2c_t *cfg, uint8_t address, uint8_t *pdata, uint16_t length);
int i2c_receive(i2c_t *cfg, uint8_t address, uint8_t *pdata, uint16_t length);

#endif  // I2C_H
