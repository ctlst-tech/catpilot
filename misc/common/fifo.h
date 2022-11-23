#ifndef FIFO_H
#define FIFO_H

#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include "os.h"

#define FIFO_MAX_SIZE 1024

typedef struct {
    uint32_t size;
    SemaphoreHandle_t queue;
    SemaphoreHandle_t write_mutex;
    SemaphoreHandle_t read_mutex;
    uint32_t count;
} fifo_t;

fifo_t *fifo_init(uint32_t size);
int fifo_write(fifo_t *fifo, uint8_t *buf, uint16_t length);
int fifo_read(fifo_t *fifo, uint8_t *buf, uint16_t length);
int fifo_get_data_size(fifo_t *fifo);

#endif  // FIFO_H
