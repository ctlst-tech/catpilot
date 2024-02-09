#include "core.h"
#include "hal.h"
#include "board.h"

usb_t usb0 = {
    .name = "ttyACM0",
    .alt_name = "COM",
    .gpio_tx = &gpio_otg_fs_dp,
    .gpio_rx = &gpio_otg_fs_dm,
};
