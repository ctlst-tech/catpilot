#include <pthread.h>

#include "board.h"
#include "core.h"
#include "fatfs.h"
#include "hal.h"
#include "log.h"
#include "periph.h"
#include "cli.h"
#include "os.h"

int board_clock_init(void);
int board_monitor_init(void);
int board_cli_init(void *dev, char *hash, char *state);
int board_periph_init(void);
int board_fs_init(void);
int board_services_start(void);

uint32_t rcc_system_clock = 400000000;
uint32_t *board_monitor_counter;
static FATFS fs;

void board_start_thread(void *param);
extern void *catpilot(void *param);

int board_start(void) {
    HAL_Init();
    board_clock_init();
    board_monitor_init();
    xTaskCreate(board_start_thread, "board_start_thread", 100, NULL, 3, NULL);
    vTaskStartScheduler();
    return 0;
}

int board_init(char *hash, char *state) {
    if (board_cli_init(&usart3, hash, state)) {
        return -1;
    }
    if (board_fs_init()) {
        return -1;
    }
    if (board_periph_init()) {
        return -1;
    }
    if (board_services_start()) {
        return -1;
    }
    return 0;
}

void board_start_thread(void *param) {
    pthread_t tid;
    pthread_attr_t attr;
    int arg = 0;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 65535);
    pthread_create(&tid, &attr, catpilot, &arg);
    pthread_join(tid, NULL);
    pthread_exit(NULL);
}

int board_cli_init(void *dev, char *hash, char *state) {
    usart_t *cli = (usart_t *)dev;

    if (usart_init(cli)) {
        return -1;
    }
    if (std_stream_init("stdin", cli, usart_open, usart_write, usart_read)) {
        return -1;
    }
    if (std_stream_init("stdout", cli, usart_open, usart_write, usart_read)) {
        return -1;
    }
    if (std_stream_init("stderr", cli, usart_open, usart_write, usart_read)) {
        return -1;
    }
    if (cli_service_start(128, 1)) {
        return -1;
    }
    if (cli_cmd_init(hash, state)) {
        return -1;
    }

    return 0;
}

void HAL_Delay(uint32_t Delay) {
    vTaskDelay(Delay);
}

uint32_t HAL_GetTick(void) {
    return xTaskGetTickCount();
}

int board_clock_init(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    // Reset
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;
    while (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    }

    // XTAL = 24 MHz, SYSCLK = 400 MHz
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 3;
    RCC_OscInitStruct.PLL.PLLN = 100;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 8;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    while (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    }

    // AHB = 200 MHz, APB1 = APB2 = APB3 = APB4 = 100 MHz
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
    while (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    }

    // PLL2
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.PLL2.PLL2M = 2;
    PeriphClkInitStruct.PLL2.PLL2N = 30;
    PeriphClkInitStruct.PLL2.PLL2P = 4;
    PeriphClkInitStruct.PLL2.PLL2Q = 5;
    PeriphClkInitStruct.PLL2.PLL2R = 1;

    // PLL3
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    PeriphClkInitStruct.PLL3.PLL3M = 3;
    PeriphClkInitStruct.PLL3.PLL3N = 72;
    PeriphClkInitStruct.PLL3.PLL3P = 3;
    PeriphClkInitStruct.PLL3.PLL3Q = 6;
    PeriphClkInitStruct.PLL3.PLL3R = 9;

    // Use special multiplexing
    PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_HSI;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
    PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL2;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
    PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;

    while (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct)) {
    }

    return 0;
}

int board_monitor_init(void) {
    #ifdef OS_MONITOR
        if(tim_init(&tim2)) {
            return -1;
        }
        board_monitor_counter = &tim2.counter_scaled;
        tim_start(&tim2);
    #endif
    return 0;
}

int board_gpio_init(void) {
    if (gpio_init(&gpio_periph_en)) {
        return -1;
    }
    if (gpio_init(&gpio_sensors_en)) {
        return -1;
    }
    gpio_reset(&gpio_periph_en);
    gpio_set(&gpio_sensors_en);

    if (gpio_init(&gpio_fmu_pwm[0])) {
        return -1;
    }
    if (gpio_init(&gpio_fmu_pwm[1])) {
        return -1;
    }
    if (gpio_init(&gpio_fmu_pwm[2])) {
        return -1;
    }
    if (gpio_init(&gpio_fmu_pwm[3])) {
        return -1;
    }
    if (gpio_init(&gpio_fmu_pwm[4])) {
        return -1;
    }
    if (gpio_init(&gpio_fmu_pwm[5])) {
        return -1;
    }
    if (gpio_init(&gpio_spi1_cs1)) {
        return -1;
    }
    if (gpio_init(&gpio_spi1_cs2)) {
        return -1;
    }
    if (gpio_init(&gpio_spi2_cs1)) {
        return -1;
    }
    if (gpio_init(&gpio_spi4_cs1)) {
        return -1;
    }
    if (gpio_init(&gpio_spi4_cs2)) {
        return -1;
    }
    if (gpio_init(&gpio_spi4_cs3)) {
        return -1;
    }
    if (gpio_init(&gpio_spi4_cs4)) {
        return -1;
    }
    gpio_set(&gpio_spi1_cs1);
    gpio_set(&gpio_spi1_cs2);
    gpio_set(&gpio_spi2_cs1);
    gpio_set(&gpio_spi4_cs1);
    gpio_set(&gpio_spi4_cs2);
    gpio_set(&gpio_spi4_cs3);
    gpio_set(&gpio_spi4_cs4);

    gpio_reset(&gpio_fmu_pwm[0]);
    gpio_reset(&gpio_fmu_pwm[1]);
    gpio_reset(&gpio_fmu_pwm[2]);
    gpio_reset(&gpio_fmu_pwm[3]);
    gpio_reset(&gpio_fmu_pwm[4]);
    gpio_reset(&gpio_fmu_pwm[5]);

    return 0;
}

int board_periph_init(void) {
    if (board_gpio_init()) {
        LOG_ERROR("GPIO", "Initialization failed");
        return -1;
    }
    if (usart_init(&usart2)) {
        LOG_ERROR("USART2", "Initialization failed");
        return -1;
    }
    if (usart_init(&usart3)) {
        LOG_ERROR("USART3", "Initialization failed");
        return -1;
    }
    if (usart_init(&usart4)) {
        LOG_ERROR("USART4", "Initialization failed");
        return -1;
    }
    if (usart_init(&usart6)) {
        LOG_ERROR("USART6", "Initialization failed");
        return -1;
    }
    if (usart_init(&usart7)) {
        LOG_ERROR("USART7", "Initialization failed");
        return -1;
    }
    if (usart_init(&usart8)) {
        LOG_ERROR("USART8", "Initialization failed");
        return -1;
    }
    if (spi_init(&spi1)) {
        LOG_ERROR("SPI1", "Initialization failed");
        return -1;
    }
    if (spi_init(&spi4)) {
        LOG_ERROR("SPI4", "Initialization failed");
        return -1;
    }
    LOG_INFO("BOARD", "Initialization successful");
    return 0;
}

static int board_sd_card_init(void) {
    struct file_operations f_op = {.open = fatfs_open,
                                   .write = fatfs_write,
                                   .read = fatfs_read,
                                   .close = fatfs_close,
                                   .fsync = fatfs_syncfs,
                                   .dev = &sdio};
    char name[] = "SDCARD";
    if (sdcard_start(name, &sdio) == NULL) {
        LOG_ERROR(name, "Initialization failed");
        return -1;
    }
    if (node_mount("/fs", &f_op) == NULL) {
        LOG_ERROR(name, "Node mount failed");
        return -1;
    }
    if (f_mount(&fs, "/", 1)) {
        LOG_ERROR(name, "f_mount error");
        return -1;
    }
    return 0;
}

int board_fs_init(void) {
    if (sdio_init(&sdio)) {
        LOG_ERROR("SDIO", "Initialization failed");
        return -1;
    }
    if (board_sd_card_init()) {
        return -1;
    }
    return 0;
}

int board_services_start(void) {
    icm20649 = icm20649_start("ICM20649", 2, 20, &spi1, &gpio_spi1_cs1,
                              &exti_spi1_drdy1);
    icm20602 = icm20602_start("ICM20602", 2, 20, &spi4, &gpio_spi4_cs2, NULL);
    icm20948 =
        icm20948_start("ICM20948", 2, 20, &spi4, &gpio_spi4_cs1, NULL, 0);
    // ms5611_2 = ms5611_start(&spi4, &gpio_spi4_cs3, 10, 8);
    cubeio = cubeio_start("CUBEIO", 2, 19, &usart6);
    ms5611_1 = ms5611_start("MS5611", 100, 18, &spi1, &gpio_spi1_cs2);
    return 0;
}

int board_fail(void) {
    LOG_ERROR("BOARD", "Fatal error");
    while (1) {
        vTaskDelay(1000);
    }
    return -1;
}
