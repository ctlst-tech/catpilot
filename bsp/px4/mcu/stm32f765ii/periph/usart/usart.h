#pragma once
#include "stm32_base.h"
#include "gpio.h"
#include "dma.h"

#define USART_TERMIOS
#define USART_POSIX_OSA

#ifdef USART_TERMIOS
#include <termios.h>
#endif

#ifdef USART_POSIX_OSA
#include "dev_posix.h"
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
    SemaphoreHandle_t semaphore;
    SemaphoreHandle_t mutex;
    IRQn_Type IRQ;
    enum usart_state_t state;
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
    enum usart_receive_mode_t mode;
    struct usart_inst_t inst;
    int init;
} usart_cfg_t;


int USART_Init(usart_cfg_t *cfg);
int USART_ClockEnable(usart_cfg_t *cfg);
int USART_Transmit(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length);
int USART_Receive(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length);
int USART_TransmitReceive(usart_cfg_t *cfg, uint8_t *tx_pdata, uint8_t *rx_pdata, uint16_t tx_length, uint16_t rx_length);
int USART_EnableIRQ(usart_cfg_t *cfg);
int USART_DisableIRQ(usart_cfg_t *cfg);
int USART_Handler(usart_cfg_t *cfg);
int USART_DMA_TX_Handler(usart_cfg_t *cfg);
int USART_DMA_RX_Handler(usart_cfg_t *cfg);

#ifdef USART_TERMIOS
    int tcgetattr(int __fd, struct termios *__termios_p);
    int tcsetattr(int __fd, int __optional_actions,
             const struct termios *__termios_p);
    speed_t cfgetospeed(const struct termios *__termios_p);
    speed_t cfgetispeed(const struct termios *__termios_p);
    int cfsetospeed(struct termios *__termios_p, speed_t __speed);
    int cfsetispeed(struct termios *__termios_p, speed_t __speed);
    int tcflush(int __fd, int __queue_selector);
#endif

#ifdef USART_POSIX_OSA
    int usart_posix_open(const char *pathname, int flags);
    ssize_t usart_posix_write(int fd, const void *buf, size_t count);
    ssize_t usart_posix_read(int fd, const void *buf, size_t count);
    int usart_posix_close(int fd);
#endif
