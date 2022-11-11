#ifndef SPI_H
#define SPI_H

#include <errno.h>

#include "core.h"
#include "dma.h"
#include "gpio.h"
#include "hal.h"

enum spi_state_t {
    SPI_FREE,
    SPI_TRANSMIT,
    SPI_RECEIVE,
    SPI_TRANSMIT_RECEIVE,
};

typedef struct {
    IRQn_Type id;
    SemaphoreHandle_t sem;
    SemaphoreHandle_t mutex;
    SemaphoreHandle_t cs_mutex;
    enum spi_state_t state;
} spi_private_t;

typedef struct {
    SPI_HandleTypeDef init;
    gpio_t *mosi;
    gpio_t *miso;
    gpio_t *sck;
    dma_t dma_tx;
    dma_t dma_rx;
    int timeout;
    int irq_priority;
    spi_private_t p;
} spi_t;

int spi_init(spi_t *cfg);
int spi_chip_select(spi_t *cfg, gpio_t *cs);
int spi_chip_deselect(spi_t *cfg, gpio_t *cs);
int spi_transmit(spi_t *cfg, uint8_t *pdata, uint16_t length);
int spi_receive(spi_t *cfg, uint8_t *pdata, uint16_t length);
int spi_transmit_receive(spi_t *cfg, uint8_t *tdata, uint8_t *rdata,
                         uint16_t length);

#endif  // SPI_H
