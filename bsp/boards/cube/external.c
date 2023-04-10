#include "board.h"

uint32_t rcc_system_clock = 400000000;
uint32_t *board_monitor_counter;
int board_app_status = 0;

void HAL_Delay(uint32_t Delay) {
    vTaskDelay(Delay);
}

uint32_t HAL_GetTick(void) {
    return xTaskGetTickCount();
}

const char *board_get_tty_name(char *path) {
    for (int i = 0; i < BOARD_MAX_USART; i++) {
        if (usart[i] != NULL) {
            if (strncmp(path, usart[i]->name, MAX_NAME_LEN) == 0 ||
                strncmp(path, usart[i]->alt_name, MAX_NAME_LEN) == 0) {
                return usart[i]->name;
            }
        }
    }
    return NULL;
}

const char *board_get_cli_tty_name(void) {
    return board_get_tty_name(CLI_PORT);
}

void board_print_tty_name(void) {
    for (int i = 0; i < BOARD_MAX_USART; i++) {
        if (usart[i] != NULL) {
            printf("%s: %s\n", usart[i]->name, usart[i]->alt_name);
        }
    }
}

int board_get_app_status(void) {
    return board_app_status;
}

void board_run_app(void) {
    board_app_status = 1;
}

void board_reset(void) {
    NVIC_SystemReset();
}

int board_get_voltage(float *buf, int length) {
    for (uint8_t i = 0; i < length; i++) {
        buf[i] = adc_get_volt(&adc1, i);
    }
    return 0;
}
