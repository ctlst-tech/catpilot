#include "usb.h"
#include "usbd_cdc_if.h"

int usb_init(usb_t *cfg) {
    MX_USB_DEVICE_Init();
    return 0;
}

int usb_open(FILE *file, const char *path) {
    return 0;
}

ssize_t usb_write(FILE *file, const char *buf, size_t count) {

}
ssize_t usb_read(FILE *file, char *buf, size_t count);
int usb_close(FILE *file);
int usb_ioctl(FILE *file, int request, va_list args);