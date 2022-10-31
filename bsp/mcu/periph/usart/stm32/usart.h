#ifndef USART_H
#define USART_H

#include "core.h"
#include "gpio.h"
#include "hal.h"

#define USART_MAX 8

#define USART_POSIX_OSA

#ifdef USART_POSIX_OSA
    #include <node.h>
#endif

enum usart_state_t {
    USART_FREE,
    USART_TRANSMIT,
    USART_RECEIVE
};

enum usart_receive_mode_t {
    USART_TIMEOUT,
    USART_IDLE,
};

struct usart_inst_t {
    UART_HandleTypeDef USART_InitStruct;
    SemaphoreHandle_t tx_semaphore;
    SemaphoreHandle_t rx_semaphore;
    SemaphoreHandle_t tx_mutex;
    SemaphoreHandle_t rx_mutex;
    IRQn_Type IRQ;
    enum usart_state_t tx_state;
    enum usart_state_t rx_state;
    int rx_count;
    int tx_count;
    ringbuf_t *write_buf;
    ringbuf_t *read_buf;
    SemaphoreHandle_t read_semaphore;
    SemaphoreHandle_t write_semaphore;
    int error;
    bool periph_init;
    bool tasks_init;
};

typedef struct {
    USART_TypeDef *USART;
    gpio_cfg_t *gpio_tx_cfg;
    gpio_cfg_t *gpio_rx_cfg;
    dma_cfg_t *dma_tx_cfg;
    dma_cfg_t *dma_rx_cfg;
    int speed;
    int timeout;
    int priority;
    int buf_size;
    enum usart_receive_mode_t mode;
    struct usart_inst_t inst;
    int task_priority;
} usart_cfg_t;

int USART_Init(usart_cfg_t *cfg);
int USART_ClockEnable(usart_cfg_t *cfg);
int USART_Transmit(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length);
int USART_Receive(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length);
int USART_TransmitReceive(usart_cfg_t *cfg, uint8_t *tx_pdata, uint8_t *rx_pdata, uint16_t tx_length, uint16_t rx_length);

int USART_SetSpeed(usart_cfg_t *cfg, uint32_t speed);
uint32_t USART_GetSpeed(usart_cfg_t *cfg);

int USART_EnableIRQ(usart_cfg_t *cfg);
int USART_DisableIRQ(usart_cfg_t *cfg);
int USART_Handler(usart_cfg_t *cfg);
int USART_DMA_TX_Handler(usart_cfg_t *cfg);
int USART_DMA_RX_Handler(usart_cfg_t *cfg);

#ifdef USART_POSIX_OSA
    int usart_posix_open(void *devcfg, void *file, const char* pathname, int flags);
    ssize_t usart_posix_write(void *devcfg, void *file, const void *buf, size_t count);
    ssize_t usart_posix_read(void *devcfg, void *file, void *buf, size_t count);
    int usart_posix_close(void *devcfg, void *file);
#endif

#endif  // USART_H
