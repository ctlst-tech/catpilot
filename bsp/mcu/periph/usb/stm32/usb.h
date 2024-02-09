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
} usb_private_t;

typedef struct {
    const char name[MAX_NAME_LEN];
    const char alt_name[MAX_NAME_LEN];
    gpio_t *gpio_tx;
    gpio_t *gpio_rx;
} usb_t;

int usb_init(usb_t *cfg);
int usb_open(FILE *file, const char *path);
ssize_t usb_write(FILE *file, const char *buf, size_t count);
ssize_t usb_read(FILE *file, char *buf, size_t count);
int usb_close(FILE *file);
int usb_ioctl(FILE *file, int request, va_list args);

#endif  // USB
