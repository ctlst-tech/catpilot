#include "irq.h"

#define MAX_IRQ 149
#define IRQ_Handler(id) (irq[id].handler != NULL ? \
                        irq[id].handler(irq[id].area) : \
                        irq_default_handler());

static irq_t irq[MAX_IRQ];

int irq_enable(IRQn_Type id, int priority, void (*handler)(void *),
               void *area) {
    if(handler != NULL && area == NULL) {
        return EINVAL;
    }
    irq[id].handler = handler;
    irq[id].area = area;
    HAL_NVIC_SetPriority(id, priority, 0);
    HAL_NVIC_EnableIRQ(id);
    return 0;
}

int irq_disable(IRQn_Type id) {
    irq[id].handler = NULL;
    irq[id].area = NULL;
    HAL_NVIC_DisableIRQ(id);
    return 0;
}

void irq_default_handler(void) {
    while(1);
}

void SPI1_IRQHandler(void) {
    IRQ_Handler(SPI1_IRQn);
}

void DMA2_Stream3_IRQHandler(void) {
    IRQ_Handler(DMA2_Stream3_IRQn);
}

void DMA2_Stream0_IRQHandler(void) {
    IRQ_Handler(DMA2_Stream0_IRQn);
}

void I2C3_EV_IRQHandler(void) {
    IRQ_Handler(I2C3_EV_IRQn);
}

void I2C3_ER_IRQHandler(void) {
    IRQ_Handler(I2C3_ER_IRQn);
}

void DMA1_Stream4_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream4_IRQn);
}

void DMA1_Stream2_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream2_IRQn);
}

void UART7_IRQHandler(void) {
    IRQ_Handler(UART7_IRQn);
}

void DMA1_Stream1_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream1_IRQn);
}

void DMA1_Stream3_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream3_IRQn);
}

void UART8_IRQHandler(void) {
    IRQ_Handler(UART8_IRQn);
}

void DMA1_Stream0_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream0_IRQn);
}

void DMA1_Stream6_IRQHandler(void) {
    IRQ_Handler(DMA1_Stream6_IRQn);
}

void SDMMC1_IRQHandler(void) {
    IRQ_Handler(SDMMC1_IRQn);
}

void DMA2_Stream6_IRQHandler(void) {
    IRQ_Handler(DMA2_Stream6_IRQn);
}

void EXTI9_5_IRQHandler(void) {
    IRQ_Handler(EXTI9_5_IRQn);
}

void EXTI4_IRQHandler(void) {
    IRQ_Handler(EXTI4_IRQn);
}
