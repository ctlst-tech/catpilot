#ifndef SPI_H
#define SPI_H

#include "core.h"
#include "gpio.h"
#include "hal.h"

enum spi_state_t {
    SPI_FREE,
    SPI_TRANSMIT,
    SPI_RECEIVE,
    SPI_TRANSMIT_RECEIVE,
};

typedef struct {
    SPI_HandleTypeDef init;
    gpio_t *mosi;
    gpio_t *miso;
    gpio_t *sck;
    dma_t *dma_mosi;
    dma_t *dma_miso;
    SemaphoreHandle_t semaphore;
    SemaphoreHandle_t mutex;
    SemaphoreHandle_t cs_mutex;
    enum spi_state_t state;
    int timeout;
    IRQn_Type irq;
    int irq_priority;
} spi_t;

int spi_init(spi_t *cfg);
int spi_clock_enable(spi_t *cfg);
int spi_chip_select(spi_t *cfg, gpio_t *cs);
int spi_chip_deselect(spi_t *cfg, gpio_t *cs);
int spi_transmit(spi_t *cfg, uint8_t *pdata, uint16_t length);
int spi_receive(spi_t *cfg, uint8_t *pdata, uint16_t length);
int spi_transmit_receive(spi_t *cfg, uint8_t *tdata, uint8_t *rdata,
                        uint16_t length);
int spi_enable_irq(spi_t *cfg);
int spi_disable_irq(spi_t *cfg);
int spi_it_handler(spi_t *cfg);
int spi_dma_mosi_handler(spi_t *cfg);
int spi_dma_miso_handler(spi_t *cfg);

#endif  // SPI_H
