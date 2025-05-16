#ifndef RING_BUF_H
#define RING_BUF_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"
#include "os.h"

#define RINGBUFFER_MAX_SIZE 2048

typedef struct {
    uint8_t *write_ptr;
    uint8_t *read_ptr;
    uint8_t *start_ptr;
    uint16_t size;
    uint16_t count;
    SemaphoreHandle_t cont_mutex;
    SemaphoreHandle_t rw_mutex;
    SemaphoreHandle_t r_sem;
    SemaphoreHandle_t w_sem;
} ring_buf_t;

ring_buf_t *ring_buf_init(uint16_t size);
uint16_t ring_buf_get_data_size(ring_buf_t *ring_buf);
uint16_t ring_buf_get_free_size(ring_buf_t *ring_buf);
int ring_buf_write(ring_buf_t *ring_buf, uint8_t *buf, uint16_t length);
int ring_buf_read(ring_buf_t *ring_buf, uint8_t *buf, uint16_t length);
int ring_buf_read_timeout(ring_buf_t *ring_buf, uint8_t *buf, uint32_t length,
                          uint32_t timeout);

#endif  // RING_BUF_H
