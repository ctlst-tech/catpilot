#include "usb.h"

#include "usb_device.h"
#include "usbd_cdc_if.h"

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

int usb_init(usb_t *cfg) {
    MX_USB_DEVICE_Init();
    return 0;
}

int usb_open(FILE *file, const char *path) {
    return 0;
}

ssize_t usb_write(FILE *file, const char *buf, size_t count) {
    CDC_Transmit_FS(buf, count);
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
    printf("USB CDC failed\n");
    return;
}

void OTG_FS_IRQHandler(void) {
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

void OTG_FS_EP1_OUT_IRQHandler(void) {
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

void OTG_FS_EP1_IN_IRQHandler(void) {
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}
