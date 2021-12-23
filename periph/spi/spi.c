#include "spi.h"

int SPI_Init(spi_cfg_t *cfg) {

    portENTER_CRITICAL();

    int rv = 0;
    if((rv = SPI_ClockEnable(cfg)) != 0) return rv;
    
    if((rv = GPIO_Init(cfg->miso_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->mosi_cfg)) != 0) return rv;

    cfg->inst.SPI_InitStruct.Instance = cfg->SPI;
    cfg->inst.SPI_InitStruct.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    cfg->inst.SPI_InitStruct.Init.CLKPhase = SPI_PHASE_1EDGE;

    if(HAL_SPI_Init(&cfg->inst.SPI_InitStruct) != HAL_OK) return EINVAL;
    
    if(cfg->inst.mutex == NULL) cfg->inst.mutex = xSemaphoreCreateMutex();
    if(cfg->inst.semaphore == NULL) cfg->inst.semaphore = xSemaphoreCreateBinary();

    portEXIT_CRITICAL();
    
    return rv;
}

int SPI_ReInit(spi_cfg_t *cfg) {
    if(HAL_SPI_DeInit(&cfg->inst.SPI_InitStruct) != HAL_OK) return EINVAL;
    if(HAL_SPI_Init(&cfg->inst.SPI_InitStruct) != HAL_OK) return EINVAL;
    return 0;
}

int SPI_Transmit(spi_cfg_t *cfg, uint8_t *pdata, uint16_t length) {
    return 0;
}

int SPI_Receive(spi_cfg_t *cfg, uint8_t *pdata, uint16_t length) {
    return 0;
}

int SPI_Handler(spi_cfg_t *cfg) {
    return 0;
}

int SPI_EnableIRQ(spi_cfg_t *cfg) {
    HAL_NVIC_SetPriority(cfg->inst.IRQ, cfg->priority, 0);
    HAL_NVIC_EnableIRQ(cfg->inst.IRQ);
}

int SPI_DisableIRQ(spi_cfg_t *cfg)  {
    HAL_NVIC_DisableIRQ(cfg->inst.IRQ);
}

int SPI_ClockEnable(spi_cfg_t *cfg) {
    switch((uint32_t)(cfg->SPI)) {

#ifdef USART1
        case SPI1_BASE:
            __HAL_RCC_SPI1_CLK_ENABLE();
            cfg->inst.IRQ = SPI1_IRQn;
            break;
#endif

#ifdef USART2
        case USART2_BASE:
            __HAL_RCC_SPI2_CLK_ENABLE();
            cfg->inst.IRQ = SPI2_IRQn;
            break;
#endif

#ifdef USART3
        case USART3_BASE:
            __HAL_RCC_SPI3_CLK_ENABLE();
            cfg->inst.IRQ = SPI3_IRQn;
            break;
#endif

#ifdef SPI4
        case SPI4_BASE:
            __HAL_RCC_SPI4_CLK_ENABLE();
            cfg->inst.IRQ = SPI4_IRQn;
            break;
#endif

#ifdef SPI5
        case SPI5_BASE:
            __HAL_RCC_SPI5_CLK_ENABLE();
            cfg->inst.IRQ = SPI5_IRQn;
            break;
#endif

#ifdef SPI6
        case SPI6_BASE:
            __HAL_RCC_SPI6_CLK_ENABLE();
            cfg->inst.IRQ = SPI6_IRQn;
            break;
#endif

    default:
        return EINVAL;
    }

    return 0;
}
