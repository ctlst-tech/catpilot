#include "board.h"
#include "core.h"
#include "hal.h"
#include "periph.h"
#include "log.h"

uint32_t rcc_system_clock = 400000000;

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

    gpio_set(&gpio_periph_en);
    gpio_reset(&gpio_sensors_en);

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

    return 0;
}

int board_services_start(void) {
    return 0;
}
