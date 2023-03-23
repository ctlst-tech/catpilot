#ifndef CAN_H
#define CAN_H

#include <errno.h>
#include <stdarg.h>

#include "core.h"
#include "dma.h"
#include "gpio.h"
#include "hal.h"
#include "ring_buf.h"

#define CAN_IOCTL_SET_ID 0x01

#define CAN_VERBOSITY_OFF 0
#define CAN_VERBOSITY_LOW 1
#define CAN_VERBOSITY_HIGH 2

#define CAN_MAX_CHANNELS 32

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
    uint32_t id;
    uint32_t frame_type;
    uint32_t size;
} can_header_t;

typedef struct {
    void *can;
    char *channel_name;
    uint32_t id;
    QueueHandle_t tx_queue;
    QueueHandle_t rx_queue;
} can_channel_t;

typedef struct {
    can_header_t header;
    uint8_t data[8];
    can_channel_t *channel;
} can_frame_t;

typedef struct {
    IRQn_Type it0_id;
    IRQn_Type it1_id;
    SemaphoreHandle_t tx_sem;
    SemaphoreHandle_t rx_sem;
    SemaphoreHandle_t tx_mutex;
    SemaphoreHandle_t rx_mutex;
    enum can_state_t state;
    bool periph_init;
    int error;
    can_channel_t *channel[CAN_MAX_CHANNELS];
    SemaphoreHandle_t channel_mutex;
    QueueHandle_t tx_queue;
} can_private_t;

typedef struct {
    const char name[MAX_NAME_LEN];
    const char alt_name[MAX_NAME_LEN];
    FDCAN_HandleTypeDef init;
    gpio_t *tx;
    gpio_t *rx;
    int timeout;
    int irq_priority;
    int task_priority;
    int queue_size;
    int verbosity;
    can_private_t p;
} can_t;

int can_init(can_t *cfg);
int can_transmit(can_t *cfg, can_header_t *header, uint8_t *pdata);
int can_receive(can_t *cfg, can_header_t *header, uint8_t *pdata);
void can_print_info(can_t *cfg, can_header_t *header, uint8_t *pdata);
int can_open(FILE *file, const char *path);
ssize_t can_write(FILE *file, const char *buf, size_t count);
ssize_t can_read(FILE *file, char *buf, size_t count);
int can_close(FILE *file);
int can_ioctl(FILE *file, int request, va_list args);

#endif  // CAN_H
