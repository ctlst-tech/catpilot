#include "monitor.h"

extern uint32_t *board_monitor_counter;
extern int heap_get_total(void);
extern int heap_get_used(void);

void monitor_start_timer(void) {
    if(cli_cmd_reg("monitor", monitor_commander) == NULL) {
        return;
    }
}

uint32_t monitor_get_counter(void) {
    return (*board_monitor_counter);
}

static char stat_buffer[1024];

int monitor_commander(int argc, char **argv) {
    (void)argc;
    (void)argv;
    int total = heap_get_total();
    int used = heap_get_used();
    int free = total - used;
    vTaskGetRunTimeStats(stat_buffer);
    printf("------------------------ Threads ------------------------\n");
    printf("%s", stat_buffer);
    printf("------------------------ Memory -------------------------\n");
    printf("Total RAM\t\t%d\n", total);
    printf("Used RAM\t\t%d\n", used);
    printf("Free RAM\t\t%d\n", free);
    printf("---------------------------------------------------------\n");
    return 0;
}
