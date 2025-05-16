#include <pthread.h>
#include <sys/ioctl.h>

#include "board.h"
#include "cli.h"
#include "core.h"
#include "fatfs.h"
#include "file.h"
#include "hal.h"
#include "log.h"
#include "os.h"
#include "periph.h"
#include "serial_bridge.h"
#include "task.h"

// Add timeout detection
static uint32_t last_task_switch_time = 0;
static const uint32_t TASK_SWITCH_TIMEOUT_MS = 5000; // 5 seconds timeout

static void check_task_switches(void) {
    static uint32_t last_total_switches = 0;
    uint32_t current_switches = xTaskGetSchedulerState();
    
    if (current_switches == last_total_switches) {
        if ((xTaskGetTickCount() - last_task_switch_time) > pdMS_TO_TICKS(TASK_SWITCH_TIMEOUT_MS)) {
            printf("WARNING: No task switches for %lu ms!\n", TASK_SWITCH_TIMEOUT_MS);
            // You could trigger system reset here
            // NVIC_SystemReset();
        }
    } else {
        last_total_switches = current_switches;
        last_task_switch_time = xTaskGetTickCount();
    }
}

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
int board_init(char *cli_port, char *baudrate);
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
    board_settings_t *bs = (board_settings_t *)arg;
    if (board_cli_init(bs->cli_port, bs->cli_baudrate)) {
        goto idle;
    }
    if (board_init(bs->cli_port, bs->cli_baudrate)) {
        fprintf(stderr, "----------------------------------------\n");
        fprintf(stderr, "FATAL ERROR: Board initialization failed\n");
        fprintf(stderr, "----------------------------------------\n");
        goto idle;
    }
    if (bs->callback()) {
        fprintf(stderr, "----------------------------------------\n");
        fprintf(stderr, "FATAL ERROR: Application start failed \n");
        fprintf(stderr, "----------------------------------------\n");
        goto idle;
    }

idle:
    while (1) {
        printf("IDLE\n");
        sleep(1);
    }
}

int board_init(char *cli_port, char *baudrate) {
    static volatile uint32_t uptime_counter = 0;
    if (board_fs_init()) {
        return -1;
    }

    if (log_init("log", "/fs/logs", LOG_TO_BUF, 1024)) {
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
        uptime_counter+=1;
        
        // FreeRTOS Debug Info
        uint32_t free_heap = xPortGetFreeHeapSize();
        UBaseType_t tasks_count = uxTaskGetNumberOfTasks();
        
        printf("====== System Status [%lu s] ======\n", uptime_counter);
        printf("Free heap: %lu bytes\n", free_heap);
        printf("Active tasks: %lu\n", tasks_count);

        // UART Status
        char *uart_states[] = {"RESET", "READY", "BUSY_TX", "BUSY_RX", "BUSY", "TIMEOUT", "ERROR"};
        usart_t *uarts[] = {&usart3, &usart4, &usart6, &usart7, &usart8};
        char *uart_names[] = {"USART3", "USART4", "USART6", "USART7", "USART8"};

        printf("\n=== UART Status ===\n");
        for (int i = 0; i < 5; i++) {
            printf("%s:\n", uart_names[i]);
            printf("  State: %s\n", uart_states[uarts[i]->init.gState & 0x0F]);
            printf("  RX State: %s\n", uart_states[uarts[i]->init.RxState & 0x0F]);
            printf("  TX pending: %lu\n", uarts[i]->init.TxXferCount);
            printf("  RX pending: %lu\n", uarts[i]->init.RxXferCount);
            printf("  Errors: 0x%lx\n", uarts[i]->init.ErrorCode);
        }

        // Task Stack Usage
        printf("\n=== Task Stack Usage ===\n");
        char task_list[500];
        vTaskList(task_list);
        printf("Task          State  Priority  Stack   Num\n");
        printf("*******************************************\n");
        printf("%s\n", task_list);

        // Add watchdog reset here if needed
        sleep(1);
    }
    return 0;
}

int board_cli_init(char *cli_port, char *baudrate) {
    periph_base_t *cli = NULL;

    int baudrate_cmd = atoi(baudrate);

    for (int i = 0; i < BOARD_MAX_CLI_DEVICES; i++) {
        if (cli_dev[i] != NULL) {
            if (!strncmp(cli_port, cli_dev[i]->name, MAX_NAME_LEN) ||
                !strncmp(cli_port, cli_dev[i]->alt_name, MAX_NAME_LEN)) {
                cli = cli_dev[i];
            }
        }
    }
    if (cli == NULL) {
        cli = (periph_base_t *)&usart3;
    }

    // TODO: add main init handler
    if (strstr(cli->name, "ttyS")) {
        usart_t *cli_usart = (usart_t *)cli;
        cli_usart->init.Init.BaudRate = (uint32_t)baudrate_cmd;
        if (usart_init(cli_usart)) {
            return -1;
        }
        cli_usart->p.stdio = true;
    } else if (strstr(cli->name, "ttyUSB")) {
        usb_t *cli_usb = (usb_t *)cli;
//        if (usb_init(cli_usb)) {
//            return -1;
//        }
        cli_usb->p.stdio = true;
    }

    if (std_stream_init("stdin", &cli->fops)) {
        return -1;
    }
    if (std_stream_init("stdout", &cli->fops)) {
        return -1;
    }
    if (std_stream_init("stderr", &cli->fops)) {
        return -1;
    }
    // cli->p.stdio = false;
    if (cli_service_start(CLI_MAX_CMD_LENGTH, 10, 1)) {
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
    RCC_OscInitStruct.PLL.PLLR = 2;
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
    PeriphClkInitStruct.PLL3.PLL3M = 6;
    PeriphClkInitStruct.PLL3.PLL3N = 72;
    PeriphClkInitStruct.PLL3.PLL3P = 3;
    PeriphClkInitStruct.PLL3.PLL3Q = 6;
    PeriphClkInitStruct.PLL3.PLL3R = 9;

    // Use special multiplexing
    PeriphClkInitStruct.PeriphClockSelection =
        RCC_PERIPHCLK_I2C123 | RCC_PERIPHCLK_SPI123 | RCC_PERIPHCLK_SPI45 |
        RCC_PERIPHCLK_USB | RCC_PERIPHCLK_ADC | RCC_PERIPHCLK_SDMMC |
        RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_HSI;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
    PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL2;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
    PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_HSE;

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
   
    return 0;
}

static int board_periph_init(void) {
    if (board_gpio_init()) {
        LOG_ERROR("GPIO", "Initialization failed");
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
        return -1;
    }
    if (node_mount("/fs", &f_op) == NULL) {
        return -1;
    }
    if (f_mount(&fs, "/", 1)) {
        return -1;
    }
    return 0;
}

static int board_fs_init(void) {
    if (sdio_init(&sdio)) {
        fprintf(stderr, "SDIO initialization failed\n");
        return -1;
    }
    if (board_sd_card_init()) {
        fprintf(stderr, "SD card initialization failed\n");
        return -1;
    }
    return 0;
}

static int board_services_start(void) {
    return 0;
}
