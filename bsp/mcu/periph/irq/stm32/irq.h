#ifndef IRQ_H
#define IRQ_H

#include <errno.h>

#include "core.h"
#include "hal.h"
#include "os.h"

typedef struct {
    void (*handler)(void *p);
    void *area;
    int priority;
} irq_t;

int irq_init(IRQn_Type id, int priority, void (*handler)(void *), void *area);
int irq_enable(IRQn_Type id);
int irq_disable(IRQn_Type id);

#endif  // IRQ_H
