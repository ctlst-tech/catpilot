#include "board.h"
#include "core.h"
#include "hal.h"
#include "icm20649.h"
#include "icm20602.h"
#include "icm20948.h"
#include "log.h"
#include "periph.h"
#include "fatfs.h"

uint32_t rcc_system_clock = 400000000;

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
    while (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK);

    // XTAL = 24 MHz, SYSCLK = 400 MHz
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 3;
    RCC_OscInitStruct.PLL.PLLN = 100;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 8;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    while (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK);

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
    while (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK);

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

    while (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct));

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
    if (sdio_init(&sdio)) {
        LOG_ERROR("SDIO", "Initialization failed");
        return -1;
    }

    return 0;
}

static int board_std_stream_init(
    const char *stream, void *dev,
    int (*dev_open)(struct file *file, const char *path),
    ssize_t (*dev_write)(struct file *file, const char *buf, size_t count),
    ssize_t (*dev_read)(struct file *file, char *buf, size_t count)) {
    int fd;
    char stream_name[16];
    char stream_path[16];
    struct file_operations f_op = {0};

    if (stream == NULL || dev == NULL || dev_open == NULL ||
        dev_write == NULL || dev_read == NULL) {
        return -1;
    }

    f_op.open = dev_open;
    f_op.dev = dev;

    if (!strcmp(stream, "stdin")) {
        f_op.read = dev_read;
    } else if (!strcmp(stream, "stdout") || !strcmp(stream, "stderr")) {
        f_op.write = dev_write;
    } else {
        return -1;
    }

    strncpy(stream_name, stream, sizeof(stream_name));
    snprintf(stream_path, sizeof(stream_path), "/dev/%s", stream_name);

    if (node_mount(stream_path, &f_op) == NULL) {
        LOG_ERROR(stream_name, "Initialization failed");
        return -1;
    }

    fd = open((const char *)stream_path, O_RDONLY);

    if (fd < 0) {
        LOG_ERROR(stream_name, "Error descriptor");
        return -1;
    }

    return 0;
}

static FATFS fs;
static int board_sd_card_init(void) {
    struct file_operations f_op = {
        .open = fatfs_open,
        .write = fatfs_write,
        .read = fatfs_read,
        .close = fatfs_close,
        .fsync = fatfs_syncfs,
        .dev = &sdio
    };
    if (node_mount("/fs", &f_op) == NULL) {
        LOG_ERROR("SDCARD", "Initialization failed");
        return -1;
    }
    if(f_mount(&fs, "/", 1)) {
        LOG_ERROR("SDMMC", "Mount error");
        return -1;
    }
    LOG_INFO("SDMMC", "Mount successful");
    return 0;
}

int board_fs_init(void) {
    if (board_std_stream_init("stdin", &usart3, usart_open, usart_write,
                              usart_read)) {
        return -1;
    }
    if (board_std_stream_init("stdout", &usart3, usart_open, usart_write,
                              usart_read)) {
        return -1;
    }
    if (board_std_stream_init("stderr", &usart3, usart_open, usart_write,
                              usart_read)) {
        return -1;
    }
    if (board_sd_card_init()) {
        return -1;
    }

    return 0;
}

int board_services_start(void) {
    icm20649_start(&spi1, &gpio_spi1_cs1, &exti_spi1_drdy1, 2, 10);
    icm20602_start(&spi4, &gpio_spi4_cs2, NULL, 2, 10);
    icm20948_start(&spi4, &gpio_spi4_cs1, NULL, 2, 10, 0);
    return 0;
}
