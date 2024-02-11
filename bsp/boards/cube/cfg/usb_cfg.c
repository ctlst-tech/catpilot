#include "core.h"
#include "hal.h"
#include "board.h"

usb_t usb0 = {
    .name = "ttyUSB0",
    .alt_name = "COM0",
    .gpio_tx = &gpio_otg_fs_dp,
    .gpio_rx = &gpio_otg_fs_dm,
    .gpio_vbus = &gpio_otg_fs_vbus,
    .gpio_id = &gpio_otg_fs_id,
    .gpio_sof = &gpio_otg_fs_sof,
    .buf_size = 512,
    .irq_priority = 10,
    .task_priority = 2,
    .read_timeout = 100,
    .p = {0}
};
