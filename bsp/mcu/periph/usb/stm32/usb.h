#ifndef USB
#define USB

#include <errno.h>

#include "core.h"
#include "dma.h"
#include "gpio.h"
#include "irq.h"
#include "node.h"
#include "os.h"
#include "ring_buf.h"

typedef struct {
    IRQn_Type id;
    SemaphoreHandle_t tx_sem;
    SemaphoreHandle_t rx_sem;
    SemaphoreHandle_t tx_mutex;
    SemaphoreHandle_t rx_mutex;
    ring_buf_t *write_buf;
    ring_buf_t *read_buf;
    uint8_t *rx_buf;
    uint8_t *tx_buf;
    int error;
    bool periph_init;
    bool tasks_init;
    bool port_open;
    bool stdio;
    int rx_count;
    int tx_count;
} usb_private_t;

typedef struct {
    const char name[MAX_NAME_LEN];
    const char alt_name[MAX_NAME_LEN];
    struct file_operations fops;
    gpio_t *gpio_tx;
    gpio_t *gpio_rx;
    gpio_t *gpio_vbus;
    gpio_t *gpio_id;
    gpio_t *gpio_sof;
    int buf_size;
    int irq_priority;
    int task_priority;
    int read_timeout;
    usb_private_t p;
} usb_t;

int usb_init(usb_t *cfg);
int usb_open(FILE *file, const char *path);
ssize_t usb_write(FILE *file, const char *buf, size_t count);
ssize_t usb_read(FILE *file, char *buf, size_t count);
int usb_close(FILE *file);
int usb_ioctl(FILE *file, int request, va_list args);

#endif  // USB
