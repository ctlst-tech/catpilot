#include "core.h"
#include "hal.h"
#include "board.h"

periph_base_t *cli_dev[BOARD_MAX_CLI_DEVICES] = {
    (periph_base_t *)&usart2,
    (periph_base_t *)&usart3,
    (periph_base_t *)&usart4,
    (periph_base_t *)&usart6,
    (periph_base_t *)&usart7,
    (periph_base_t *)&usart8,
    (periph_base_t *)&usb0,
};
