#ifndef CAN_H
#define CAN_H

#include <errno.h>

#include "core.h"
#include "dma.h"
#include "gpio.h"
#include "hal.h"
#include "ring_buf.h"

#define CAN_VERBOSITY_OFF  0
#define CAN_VERBOSITY_LOW  1
#define CAN_VERBOSITY_HIGH 2

enum can_state_t {
    CAN_FREE = 0,
    CAN_TRANSMIT = 1,
    CAN_RECEIVE = 2,
};

enum can_frame_type_t {
    CAN_DATA_FRAME = 0,
    CAN_REMOTE_FRAME = 1,
};

typedef struct {
    IRQn_Type it0_id;
    IRQn_Type it1_id;
    SemaphoreHandle_t sem;
    SemaphoreHandle_t mutex;
    enum can_state_t state;
    ring_buf_t *read_buf;
    ring_buf_t *write_buf;
    bool periph_init;
} can_private_t;

typedef struct {
    uint32_t id;
    uint32_t frame_type;
    uint32_t size;
} can_header_t;

typedef struct {
    const char name[MAX_NAME_LEN];
    const char alt_name[MAX_NAME_LEN];
    FDCAN_HandleTypeDef init;
    gpio_t *tx;
    gpio_t *rx;
    int timeout;
    int irq_priority;
    int task_priority;
    int buf_size;
    int verbosity;
    can_private_t p;
} can_t;

int can_init(can_t *cfg);
int can_transmit(can_t *cfg, can_header_t *header, uint8_t *pdata);
int can_receive(can_t *cfg, can_header_t *header, uint8_t *pdata);
void can_print_info(can_t *cfg, can_header_t *header, uint8_t *pdata);

#endif  // CAN_H
