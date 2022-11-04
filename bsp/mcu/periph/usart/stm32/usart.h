#ifndef USART_H
#define USART_H

#include "core.h"
#include "os.h"
#include "periph.h"
#include "ring_buf.h"

enum usart_state_t { USART_FREE, USART_TRANSMIT, USART_RECEIVE };

enum usart_receive_mode_t {
    USART_TIMEOUT,
    USART_IDLE,
};

typedef struct {
    IRQn_Type id;
    SemaphoreHandle_t tx_sem;
    SemaphoreHandle_t rx_sem;
    SemaphoreHandle_t tx_mutex;
    SemaphoreHandle_t rx_mutex;
    enum usart_state_t tx_state;
    enum usart_state_t rx_state;
    int rx_count;
    int tx_count;
    ring_buf_t *write_buf;
    ring_buf_t *read_buf;
    SemaphoreHandle_t read_sem;
    SemaphoreHandle_t write_sem;
    int error;
    bool periph_init;
    bool tasks_init;
} usart_private_t;

typedef struct {
    UART_HandleTypeDef init;
    dma_t dma_tx;
    dma_t dma_rx;
    enum usart_receive_mode_t mode;
    gpio_t *gpio_tx;
    gpio_t *gpio_rx;
    int buf_size;
    int timeout;
    int irq_priority;
    int task_priority;
    usart_private_t p;
} usart_t;

int usart_init(usart_t *cfg);
int usart_transmit(usart_t *cfg, uint8_t *pdata, uint16_t length);
int usart_receive(usart_t *cfg, uint8_t *pdata, uint16_t length);
int usart_transmit_receive(usart_t *cfg, uint8_t *tx_pdata, uint8_t *rx_pdata,
                           uint16_t tx_length, uint16_t rx_length);
int usart_set_speed(usart_t *cfg, uint32_t speed);
uint32_t usart_get_speed(usart_t *cfg);
int usart_posix_open(void *devcfg, void *file, const char *pathname, int flags);
ssize_t usart_posix_write(void *devcfg, void *file, const void *buf,
                          size_t count);
ssize_t usart_posix_read(void *devcfg, void *file, void *buf, size_t count);
int usart_posix_close(void *devcfg, void *file);

#endif  // USART_H
