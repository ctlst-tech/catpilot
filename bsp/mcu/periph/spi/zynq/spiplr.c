#include "spiplr.h"

#include <f/ferrors.h>
#include <hw/inout.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#define WRITE_REG(__r, __d) out32((__r), (__d))
#define READ_REG(r) in32(r)

int spiplr_init(spiplr_instance_t *i, uintptr_t phys_base) {
    i->base = mmap_device_io(SPIPLR_REG_MAP_SIZE, phys_base);
    if (i->base == NULL) {
        perror("mmap_device_io");
        return -1;
    }
    i->phys_base = phys_base;
    return 0;
}

int spiplr_poll_set_period(spiplr_instance_t *i, uint32_t period_us,
                           uint32_t mck) {
    //#warning switch back reg write
    uint32_t per_reg = mck / (1000000 / period_us);
    WRITE_REG(i->base + SPIPLR_POLL_PERIOD_OFF, per_reg);
    // printf("Per reg %08X\n", per_reg);
    // WRITE_REG(i->base + SPIPLR_POLL_PERIOD_OFF, 0x000306A0);

    return 0;
}

int spiplr_poll_start(spiplr_instance_t *i) {
    WRITE_REG(i->base + SPIPLR_CNT_OFF, SPIPLR_CNT_ENABLE);
    return 0;
}
int spiplr_poll_stop(spiplr_instance_t *i) {
    WRITE_REG(i->base + SPIPLR_CNT_OFF, SPIPLR_CNT_DISABLE);
    return 0;
}
int spiplr_reset(spiplr_instance_t *i) {
    WRITE_REG(i->base + SPIPLR_CNT_OFF, SPIPLR_CNT_RESET);
    return 0;
}

int spiplr_get_status(spiplr_instance_t *i, uint32_t *status) {
    *status = READ_REG(i->base + SPIPLR_STAT_OFF);
    return 0;
}

int spiplr_reset_status(spiplr_instance_t *i) {
    WRITE_REG(i->base + SPIPLR_STAT_OFF, 0);
    return 0;
}

int spiplr_req_vector_set(spiplr_instance_t *i, uint32_t *v, int size) {
    int j;
    if (size >= SPIPLR_POLL_VECTOR_SIZE - 1) {
        return -1;
    }

    for (j = 0; j < size; j++) {
        WRITE_REG(i->base + SPIPLR_POLL_REQ_VECTOR_OFF + (j << 2), v[j]);

        // WRITE_REG(i->base + SPIPLR_POLL_REQ_VECTOR_OFF + (size << 2) + (j <<
        // 2), v[j]);
    }

    WRITE_REG(i->base + SPIPLR_POLL_REQ_VECTOR_OFF + (j << 2),
              SPIPLR_COMPOSE_REQ_TERMINATION());
    // WRITE_REG(i->base + SPIPLR_POLL_REQ_VECTOR_OFF + (size << 2) + (j << 2),
    // SPIPLR_COMPOSE_REQ_TERMINATION());

    return 0;
}

int spiplr_resp_vector_get(spiplr_instance_t *i, uint32_t *v, int *size) {
    int j;
    int nr = (*size) >> 2;

    // printf("Response vector get\n");
    for (j = 0; j < nr; j++) {
        v[j] = READ_REG(i->base + SPIPLR_POLL_RESP_VECTOR_OFF + (j << 2));
        // printf("%d 0x%x\n", j, v[j]);
        //        if (SPIPLR_RESP_EXTRACT_DEV_ID(v[j]) == 0) {
        //            *size = j;
        //            break;
        //        }
    }
    *size = nr;

    return 0;
}

int spiplr_cfg_vector_set(spiplr_instance_t *i, uint32_t *v, int size) {
    int j;
    // if (size >= SPIPLR_CFG_VECTOR_SIZE - 1) {
    //     return -1;
    // }

    if (size > SPIPLR_CFG_VECTOR_SIZE) {
        return -1;
    }

    for (j = 0; j < size; j++) {
        WRITE_REG(i->base + SPIPLR_CFG_REQ_VECTOR_OFF + (j << 2), v[j]);
        // printf("Read value 0x%x result 0x%x\n", *(v + j), READ_REG(i->base +
        // SPIPLR_CFG_REQ_VECTOR_OFF + (j << 2)));
    }

    return 0;
}

int spiplr_irq_poll_rdy_enable(spiplr_instance_t *i) {
    WRITE_REG(i->base + SPIPLR_IRQ_OFF, SPIPLR_IRQ_POLL_RDY_EN);
    return 0;
}

int spiplr_irq_poll_rdy_disable(spiplr_instance_t *i) {
    WRITE_REG(i->base + SPIPLR_IRQ_OFF, SPIPLR_IRQ_POLL_RDY_DIS);
    return 0;
}

void spiplr_print_req_vector(uint32_t *v, int size) {
    int i;
    for (i = 0; i < size; i++) {
        printf("req_vector. 0x%08X: 0x%08X\n", i << 2, v[i]);
    }
}

int spiplr_dump_reg_map(spiplr_instance_t *inst) {
    int i;
    for (i = 0; i < SPIPLR_REG_MAP_SIZE; i += 4) {
        printf("spiplr_dump. 0x%08X: 0x%08X\n", inst->phys_base + i,
               READ_REG(inst->base + i));
    }
    return 0;
}

int spi_plr_custom_trans(spiplr_instance_t *i, uint32_t *inb, int n,
                         uint32_t *outb) {
    if (n > SPIPLR_CR_MAX_TRANS) {
        return -1;
    }

    int j;

    for (j = 0; j < n; j++) {
        WRITE_REG(i->base + SPIPLR_CR_REQ_OFF + (j << 2), inb[j]);
    }

    for (j = n; j < SPIPLR_CR_MAX_TRANS; j++) {
        WRITE_REG(i->base + SPIPLR_CR_REQ_OFF + (j << 2), 0);
    }

    j = 32000;

    WRITE_REG(i->base + SPIPLR_CNT_OFF, SPIPLR_CNT_CR_START);
    while (!(READ_REG(i->base + SPIPLR_STAT_OFF) & SPIPLR_STAT_CR_RDY)) {
        if (j-- <= 0) {
            error( "SPIPLR_STAT_CR_RDY hang recovery");
            break;
        }
    }

    spiplr_reset_status(i);

    for (j = 0; j < n; j++) {
        outb[j] = READ_REG(i->base + SPIPLR_CR_RESP_OFF + (j << 2));
    }

    return 0;
}

uint32_t spi_plr_get_irq_counter(spiplr_instance_t *i) {
    return READ_REG(i->base + SPIPLR_IRQ_COUNTER_OFF);
}

uint32_t spi_plr_get_irq_proc_time(spiplr_instance_t *i) {
    return READ_REG(i->base + SPIPLR_IRQ_PROC_TIME_OFF);
}

uint32_t spi_plr_get_poll_cycle_time(spiplr_instance_t *i) {
    return READ_REG(i->base + SPIPLR_POLL_CYCLE_TIME_OFF);
}
