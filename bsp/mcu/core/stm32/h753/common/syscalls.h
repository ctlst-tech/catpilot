#ifndef SYSCALLS_H
#define SYSCALLS_H

unsigned heap_get_total(void);
unsigned heap_get_used(void);

void heap_allocation_stat_init(void);
void heap_allocation_stat_from_prev_call(const char *header);

#endif  // SYSCALLS_H
