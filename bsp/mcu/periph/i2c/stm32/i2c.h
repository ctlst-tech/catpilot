#ifndef I2C_H
#define I2C_H

#include "core.h"
#include "dma.h"
#include "gpio.h"
#include "hal.h"
#include "os.h"

enum i2c_state_t { I2C_FREE, I2C_TRANSMIT, I2C_RECEIVE };

typedef struct {
    I2C_HandleTypeDef init;
    gpio_t *scl;
    gpio_t *sda;
    dma_t *dma_tx;
    dma_t *dma_rx;
    SemaphoreHandle_t semaphore;
    SemaphoreHandle_t mutex;
    enum i2c_state_t state;
    int timeout;
    IRQn_Type ev_irq;
    IRQn_Type er_irq;
    int irq_priority;
} i2c_t;

int i2c_init(i2c_t *cfg);
int i2c_clock_enable(i2c_t *cfg);
int i2c_transmit(i2c_t *cfg, uint8_t address, uint8_t *pdata, uint16_t length);
int i2c_receive(i2c_t *cfg, uint8_t address, uint8_t *pdata, uint16_t length);
int i2c_enable_irq(i2c_t *cfg);
int i2c_disable_irq(i2c_t *cfg);
int i2c_ev_handler(i2c_t *cfg);
int i2c_er_handler(i2c_t *cfg);
int i2c_dma_tx_handler(i2c_t *cfg);
int i2c_dma_rx_handler(i2c_t *cfg);

#endif  // I2C_H
