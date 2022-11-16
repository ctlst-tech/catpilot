#ifndef USART_H
#define USART_H

#include <errno.h>

#include "core.h"
#include "dma.h"
#include "gpio.h"
#include "irq.h"
#include "node.h"
#include "os.h"
#include "ring_buf.h"

enum usart_state_t { USART_FREE = 0, USART_TRANSMIT = 1, USART_RECEIVE = 2 };

enum usart_receive_mode_t {
    USART_TIMEOUT = 0,
    USART_IDLE = 1,
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
    int error;
    bool periph_init;
    bool tasks_init;
    bool use_dma;
} usart_private_t;

typedef struct {
    const char name[10];
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
int usart_open(FILE *file, const char *path);
ssize_t usart_write(FILE *file, const char *buf, size_t count);
ssize_t usart_read(FILE *file, char *buf, size_t count);
int usart_close(FILE *file);

#endif  // USART_H
