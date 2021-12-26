#pragma once
#include "stm32_base.h"
#include "gpio.h"
#include "dma.h"

enum usart_state_t {
    USART_FREE,
    USART_TRANSMIT,
    USART_RECEIVE
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
    struct usart_inst_t inst;
} usart_cfg_t;


int USART_Init(usart_cfg_t *cfg);
int USART_ClockEnable(usart_cfg_t *cfg);
int USART_Transmit(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length);
int USART_Receive(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length);
int USART_EnableIRQ(usart_cfg_t *cfg);
int USART_DisableIRQ(usart_cfg_t *cfg); 
int USART_Handler(usart_cfg_t *cfg);
