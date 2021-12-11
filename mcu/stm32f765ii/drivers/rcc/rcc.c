#include "rcc.h"

void RCC_Init() {
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PLLSAIInitTypeDef RCC_PLLSAIInitStruct;
    RCC_PLLI2SInitTypeDef RCC_PLLI2SInitStruct;

    // XTAL = 16 MHz, SYSCLK = 216 MHz
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 216;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 9;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    while(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK);

    // AHB = 216 MHz, APB1 = 54 MHz, APB2 = 108 MHz
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                    | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    while(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK);

    RCC_PLLSAIInitStruct.PLLSAIN = 192;
    RCC_PLLSAIInitStruct.PLLSAIP = 8;
    RCC_PLLSAIInitStruct.PLLSAIQ = 4;
    RCC_PLLSAIInitStruct.PLLSAIR = 2;
    while(HAL_RCCEx_EnablePLLI2S(&RCC_PLLI2SInitStruct) != HAL_OK);
    
    RCC_PLLI2SInitStruct.PLLI2SN = 192;
    RCC_PLLI2SInitStruct.PLLI2SP = 2;
    RCC_PLLI2SInitStruct.PLLI2SQ = 2;
    RCC_PLLI2SInitStruct.PLLI2SR = 2;
    while(HAL_RCCEx_EnablePLLI2S(&RCC_PLLI2SInitStruct) != HAL_OK);
}