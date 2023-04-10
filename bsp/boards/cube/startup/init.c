#include <pthread.h>
#include <sys/ioctl.h>

#include "board.h"
#include "cli.h"
#include "core.h"
#include "fatfs.h"
#include "hal.h"
#include "log.h"
#include "os.h"
#include "periph.h"
#include "serial_bridge.h"

typedef struct {
    int (*callback)(void);
    size_t stacksize;
    char *cli_port;
    char *cli_baudrate;
} board_settings_t;

// Threads
void board_start_thread(void *param);
void *board_thread(void *arg);

// Private functions
static int board_clock_init(void);
static int board_monitor_init(void);
static int board_init(char *cli_port, char *baudrate);
static int board_cli_init(char *cli_port, char *baudrate);
static int board_fs_init(void);
static int board_periph_init(void);
static int board_gpio_init(void);
static int board_services_start(void);

// Private data
static board_settings_t board_settings;
static FATFS fs;

// External functions
extern int board_get_app_status(void);
extern int board_run_app(void);

int board_start(int (*callback)(void), size_t stacksize, char *cli_port,
                char *cli_baudrate) {
    HAL_Init();
    board_clock_init();
    board_monitor_init();
    board_settings.callback = callback;
    board_settings.stacksize = stacksize;
    board_settings.cli_port = cli_port;
    board_settings.cli_baudrate = cli_baudrate;
    xTaskCreate(board_start_thread, "board_start_thread",
                configMINIMAL_STACK_SIZE, &board_settings, 3, NULL);
    vTaskStartScheduler();
    return 0;
}

void board_start_thread(void *param) {
    board_settings_t *board_setting = (board_settings_t *)param;
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, board_setting->stacksize);
    pthread_create(&tid, &attr, board_thread, param);
    pthread_join(tid, NULL);
    pthread_exit(NULL);
}

void *board_thread(void *arg) {
    board_settings_t *board_settings = (board_settings_t *)arg;
    if (board_init(board_settings->cli_port, board_settings->cli_baudrate)) {
        LOG_ERROR("BOARD", "Board initialization failed");
    }
    if (board_settings->callback()) {
        LOG_ERROR("BOARD", "Application error");
    }
    while (1) {
        sleep(1);
    }
}

static int board_init(char *cli_port, char *baudrate) {
    if (board_cli_init(cli_port, baudrate)) {
        return -1;
    }
    if (board_fs_init()) {
        return -1;
    }
    if (log_init("log", "/fs/logs", LOG_TO_FILE, 512)) {
        return -1;
    }
    if (board_periph_init()) {
        return -1;
    }
    if (board_services_start()) {
        return -1;
    }
#ifndef MAINTENANCE_MODE
    board_run_app();
#endif
    while (!board_get_app_status()) {
        sleep(1);
    }
    return 0;
}

int board_cli_init(char *cli_port, char *baudrate) {
    usart_t *cli = NULL;

    int baudrate_cmd = atoi(baudrate);

    for (int i = 0; i < BOARD_MAX_USART; i++) {
        if (usart[i] != NULL) {
            if (strncmp(cli_port, usart[i]->name, MAX_NAME_LEN) == 0 ||
                strncmp(cli_port, usart[i]->alt_name, MAX_NAME_LEN) == 0) {
                cli = usart[i];
                cli->init.Init.BaudRate = (uint32_t)baudrate_cmd;
            }
        }
    }
    if (cli == NULL) {
        cli = &usart3;
    }
    if (usart_init(cli)) {
        return -1;
    }
    cli->p.stdio = true;
    if (std_stream_init("stdin", cli, usart_open, usart_write, usart_read)) {
        return -1;
    }
    if (std_stream_init("stdout", cli, usart_open, usart_write, usart_read)) {
        return -1;
    }
    if (std_stream_init("stderr", cli, usart_open, usart_write, usart_read)) {
        return -1;
    }
    cli->p.stdio = false;
    if (cli_service_start(CLI_MAX_CMD_LENGTH, 1)) {
        return -1;
    }
    if (cli_cmd_init()) {
        return -1;
    }
    return 0;
}

static int board_clock_init(void) {
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
    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;

    while (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct)) {
    }

    return 0;
}

static int board_monitor_init(void) {
#ifdef OS_MONITOR
    if (tim_init(&tim2)) {
        return -1;
    }
    board_monitor_counter = &tim2.counter;
    tim_start(&tim2);
#endif
    return 0;
}

static int board_gpio_init(void) {
    if (gpio_init(&gpio_periph_en)) {
        return -1;
    }
    if (gpio_init(&gpio_sensors_en)) {
        return -1;
    }

    gpio_reset(&gpio_sensors_en);
    gpio_set(&gpio_periph_en);
    vTaskDelay(20);
    gpio_set(&gpio_sensors_en);
    gpio_reset(&gpio_periph_en);
    vTaskDelay(20);

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
    if (gpio_init(&gpio_adc_inp4)) {
        return -1;
    }
    if (gpio_init(&gpio_adc_inp8)) {
        return -1;
    }
    if (gpio_init(&gpio_adc_inp13)) {
        return -1;
    }
    if (gpio_init(&gpio_adc_inp14)) {
        return -1;
    }
    if (gpio_init(&gpio_adc_inp15)) {
        return -1;
    }
    if (gpio_init(&gpio_adc_inp18)) {
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

static int board_periph_init(void) {
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
    if (i2c_init(&i2c1)) {
        LOG_ERROR("I2C1", "Initialization failed");
        return -1;
    }
    if (i2c_init(&i2c2)) {
        LOG_ERROR("I2C2", "Initialization failed");
        return -1;
    }
    if (adc_init(&adc1)) {
        LOG_ERROR("ADC1", "Initialization failed");
        return -1;
    }
    if (can_init(&can1)) {
        LOG_ERROR("CAN1", "Initialization failed");
        return -1;
    }
    if (can_init(&can2)) {
        LOG_ERROR("CAN2", "Initialization failed");
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
                                   .mkdir = fatfs_mkdir,
                                   .rmdir = fatfs_rmdir,
                                   .lseek = fatfs_lseek,
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

static int board_fs_init(void) {
    if (sdio_init(&sdio)) {
        LOG_ERROR("SDIO", "Initialization failed");
        return -1;
    }
    if (board_sd_card_init()) {
        return -1;
    }
    return 0;
}

static int board_services_start(void) {
    serial_bridge_start(15, 1024);
    icm20649 = icm20649_start("ICM20649", 2, 20, &spi1, &gpio_spi1_cs1,
                              &exti_spi1_drdy1);
    icm20602 = icm20602_start("ICM20602", 2, 20, &spi4, &gpio_spi4_cs2, NULL);
    icm20948 =
        icm20948_start("ICM20948", 2, 20, &spi4, &gpio_spi4_cs1, NULL, 0);
    cubeio = cubeio_start("CUBEIO", 0, 19, &usart6);
    ms5611_1 = ms5611_start("MS5611_INT", 100, 17, &spi1, &gpio_spi1_cs2);
    ms5611_2 = ms5611_start("MS5611_EXT", 100, 17, &spi4, &gpio_spi4_cs3);
    ist8310 = ist8310_start("IST8310_EXT", 100, 17, &i2c1);
    return 0;
}
