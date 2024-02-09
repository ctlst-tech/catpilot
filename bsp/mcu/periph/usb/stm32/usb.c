#include "usb.h"
#include "usbd_cdc_if.h"
#include "usb_device.h"

int usb_init(usb_t *cfg) {
    if (gpio_init(cfg->gpio_tx)) {
        return -1;
    }
    if (gpio_init(cfg->gpio_rx)) {
        return -1;
    }
    MX_USB_DEVICE_Init();
    return 0;
}

int usb_open(FILE *file, const char *path) {
    return 0;
}

ssize_t usb_write(FILE *file, const char *buf, size_t count) {
    return 0;
}

ssize_t usb_read(FILE *file, char *buf, size_t count) {
    return 0;
}

int usb_close(FILE *file) {
    return 0;
}

int usb_ioctl(FILE *file, int request, va_list args) {
    return 0;
}

void usb_error_handler() {
    return;
}

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
void OTG_FS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}
