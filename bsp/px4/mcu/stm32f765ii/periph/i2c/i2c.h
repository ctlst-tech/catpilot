#pragma once
#include "stm32_base.h"
#include "gpio.h"
#include "dma.h"

enum i2c_state_t {
    I2C_FREE,
    I2C_TRANSMIT,
    I2C_RECEIVE
};

struct i2c_inst_t {
    I2C_HandleTypeDef I2C_InitStruct;
    SemaphoreHandle_t semaphore;
    SemaphoreHandle_t mutex;
    IRQn_Type EV_IRQ;
    IRQn_Type ER_IRQ;
    enum i2c_state_t state;
};

typedef struct {
    I2C_TypeDef *I2C;
    gpio_cfg_t *scl_cfg;
    gpio_cfg_t *sda_cfg;
    dma_cfg_t *dma_tx_cfg;
    dma_cfg_t *dma_rx_cfg;
    int timeout;
    int priority;
    struct i2c_inst_t inst;
} i2c_cfg_t;

int I2C_Init(i2c_cfg_t *cfg);
int I2C_ClockEnable(i2c_cfg_t *cfg);
int I2C_Transmit(i2c_cfg_t *cfg, uint8_t address, uint8_t *pdata, uint16_t length);
int I2C_Receive(i2c_cfg_t *cfg, uint8_t address, uint8_t *pdata, uint16_t length);
int I2C_EnableIRQ(i2c_cfg_t *cfg);
int I2C_DisableIRQ(i2c_cfg_t *cfg);
int I2C_EV_Handler(i2c_cfg_t *cfg);
int I2C_ER_Handler(i2c_cfg_t *cfg);
int I2C_DMA_TX_Handler(i2c_cfg_t *cfg);
int I2C_DMA_RX_Handler(i2c_cfg_t *cfg);
