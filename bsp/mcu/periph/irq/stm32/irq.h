#ifndef IRQ_H
#define IRQ_H

#include "core.h"
#include <errno.h>
#include "hal.h"
#include "os.h"

typedef struct {
    void (*handler)(void *p);
    void *area;
} irq_t;

int irq_enable(IRQn_Type id, int priority, void (*handler)(void *), void *area);
int irq_disable(IRQn_Type id);

#endif  // IRQ_H
