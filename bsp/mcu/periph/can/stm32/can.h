#ifndef CAN_H
#define CAN_H

#include <errno.h>

#include "core.h"
#include "dma.h"
#include "gpio.h"
#include "hal.h"
#include "ring_buf.h"

enum can_state_t {
    CAN_FREE = 0,
    CAN_TRANSMIT = 1,
    CAN_RECEIVE = 2,
};

typedef struct {
    IRQn_Type it0_id;
    IRQn_Type it1_id;
    SemaphoreHandle_t sem;
    SemaphoreHandle_t mutex;
    SemaphoreHandle_t cs_mutex;
    enum can_state_t state;
    ring_buf_t *read_buf;
    ring_buf_t *write_buf;
    bool periph_init;
} can_private_t;

typedef struct {
    const char name[MAX_NAME_LEN];
    FDCAN_HandleTypeDef init;
    gpio_t *tx;
    gpio_t *rx;
    int timeout;
    int irq_priority;
    int task_priority;
    int buf_size;
    can_private_t p;
} can_t;

int can_init(can_t *cfg);
int can_transmit(can_t *cfg, uint8_t *pdata, uint16_t length);
int can_receive(can_t *cfg, uint8_t *pdata, uint16_t length);

#endif  // CAN_H
