#include "stm32_base.h"
#include <stdint.h>
#include <stdbool.h>

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
