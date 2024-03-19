#ifndef PERIPH_H
#define PERIPH_H

#include "adc.h"
#include "can.h"
#include "dma.h"
#include "exti.h"
#include "gpio.h"
#include "i2c.h"
#include "irq.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"

typedef struct {
    const char name[MAX_NAME_LEN];
    const char alt_name[MAX_NAME_LEN];
    struct file_operations fops;
    int (*init)(void *cfg);
} periph_base_t;

#endif  // PERIPH_H
