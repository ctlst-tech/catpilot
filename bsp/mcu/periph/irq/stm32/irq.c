#include "irq.h"
#define MAX_IRQ 149

#define IRQ_Handler(id) (irq[id].handler != NULL ? \
                        irq[id].handler(irq[id].area) : \
                        irq_default_handler());

static irq_t irq[MAX_IRQ];

int irq_init(IRQn_Type id, int priority, void (*handler)(void *),
               void *area) {
    if (handler != NULL && area == NULL) {
        return EINVAL;
    }
    if (irq[id].handler != NULL) {
        return EEXIST;
    }
    irq[id].handler = handler;
    irq[id].area = area;
    irq[id].priority = priority;
    return 0;
}

int irq_enable(IRQn_Type id) {
    if (irq[id].handler == NULL) {
        return EINVAL;
    }
    HAL_NVIC_SetPriority(id, irq[id].priority, 0);
    HAL_NVIC_EnableIRQ(id);
    return 0;
}

int irq_disable(IRQn_Type id) {
    HAL_NVIC_DisableIRQ(id);
    return 0;
}

void irq_default_handler(void) {
    while(1);
}

// Handlers

// ADC
void ADC_IRQHandler(void) {
    IRQ_Handler(ADC_IRQn);
}

void ADC3_IRQHandler(void) {
    IRQ_Handler(ADC3_IRQn);
}

// DMA
void DMA1_Stream0_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream0_IRQn);
}

void DMA1_Stream1_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream1_IRQn);
}

void DMA1_Stream2_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream2_IRQn);
}

void DMA1_Stream3_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream3_IRQn);
}

void DMA1_Stream4_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream4_IRQn);
}

void DMA1_Stream5_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream5_IRQn);
}

void DMA1_Stream6_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream6_IRQn);
}

void DMA1_Stream7_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream7_IRQn);
}

void DMA2_Stream0_IRQHandler(void) {
    IRQ_Handler(DMA2_Stream0_IRQn);
}

void DMA2_Stream1_IRQHandler(void) {
    IRQ_Handler(DMA2_Stream1_IRQn);
}

void DMA2_Stream2_IRQHandler(void) {
    IRQ_Handler(DMA2_Stream2_IRQn);
}

void DMA2_Stream3_IRQHandler(void) {
    IRQ_Handler(DMA2_Stream3_IRQn);
}

void DMA2_Stream4_IRQHandler(void) {
    IRQ_Handler(DMA2_Stream4_IRQn);
}

void DMA2_Stream5_IRQHandler(void) {
    IRQ_Handler(DMA2_Stream5_IRQn);
}

void DMA2_Stream6_IRQHandler(void) {
    IRQ_Handler(DMA2_Stream6_IRQn);
}

void DMA2_Stream7_IRQHandler(void) {
    IRQ_Handler(DMA2_Stream7_IRQn);
}

// I2C
void I2C1_EV_IRQHandler(void) {
    IRQ_Handler(I2C1_EV_IRQn);
}

void I2C1_ER_IRQHandler(void) {
    IRQ_Handler(I2C1_ER_IRQn);
}

void I2C2_EV_IRQHandler(void) {
    IRQ_Handler(I2C2_EV_IRQn);
}

void I2C2_ER_IRQHandler(void) {
    IRQ_Handler(I2C2_ER_IRQn);
}

void I2C3_EV_IRQHandler(void) {
    IRQ_Handler(I2C3_EV_IRQn);
}

void I2C3_ER_IRQHandler(void) {
    IRQ_Handler(I2C3_ER_IRQn);
}

// SDMMC
void SDMMC1_IRQHandler(void) {
    IRQ_Handler(SDMMC1_IRQn);
}

void SDMMC2_IRQHandler(void) {
    IRQ_Handler(SDMMC2_IRQn);
}

// SPI
void SPI1_IRQHandler(void) {
    IRQ_Handler(SPI1_IRQn);
}

void SPI2_IRQHandler(void) {
    IRQ_Handler(SPI2_IRQn);
}

void SPI3_IRQHandler(void) {
    IRQ_Handler(SPI3_IRQn);
}

void SPI4_IRQHandler(void) {
    IRQ_Handler(SPI4_IRQn);
}

void SPI5_IRQHandler(void) {
    IRQ_Handler(SPI5_IRQn);
}

void SPI6_IRQHandler(void) {
    IRQ_Handler(SPI6_IRQn);
}

// USART
void USART1_IRQHandler(void) {
    IRQ_Handler(USART1_IRQn);
}

void USART2_IRQHandler(void) {
    IRQ_Handler(USART2_IRQn);
}

void USART3_IRQHandler(void) {
    IRQ_Handler(USART3_IRQn);
}

void UART4_IRQHandler(void) {
    IRQ_Handler(UART4_IRQn);
}

void UART5_IRQHandler(void) {
    IRQ_Handler(UART5_IRQn);
}

void USART6_IRQHandler(void) {
    IRQ_Handler(USART6_IRQn);
}

void UART7_IRQHandler(void) {
    IRQ_Handler(UART7_IRQn);
}

void UART8_IRQHandler(void) {
    IRQ_Handler(UART8_IRQn);
}

void EXTI4_IRQHandler(void) {
    IRQ_Handler(EXTI4_IRQn);
}

void EXTI9_5_IRQHandler(void) {
    IRQ_Handler(EXTI9_5_IRQn);
}

void EXTI15_10_IRQHandler(void) {
    IRQ_Handler(EXTI15_10_IRQn);
}
