#include "monitor.h"

extern uint32_t *board_monitor_counter;
extern int heap_get_total(void);
extern int heap_get_used(void);

void monitor_start_timer(void) {
    if (cli_cmd_reg("monitor", monitor_commander) == NULL) {
        return;
    }
}

uint32_t monitor_get_counter(void) {
    return (*board_monitor_counter);
}

static char stat_buffer[1024];

static void monitor_print_help(void) {
    printf(
        "Usage: monitor [options]\n"
        "Options:\n"
        "\t-n\tPrint statistics continuously\n");
}

static void monitor_print_thread_stat(void) {
    printf("------------------------ Threads ------------------------\n");
    vTaskGetRunTimeStats(stat_buffer);
    printf("%s", stat_buffer);
}

static void monitor_print_mem_stat(void) {
    int total = heap_get_total();
    int used = heap_get_used();
    int free = total - used;
    printf("------------------------ Memory ------------------------\n");
    printf("Total RAM\t\t%d\n", total);
    printf("Used RAM\t\t%d\n", used);
    printf("Free RAM\t\t%d\n", free);
}

static void monitor_print_all(void) {
    monitor_print_thread_stat();
    monitor_print_mem_stat();
    // debug
    printf("counter = %d\n", *board_monitor_counter);
}

static void monitor_print_non_stop() {
    while (1) {
        monitor_print_all();
        vTaskDelay(1000);
    }
}

int monitor_commander(int argc, char **argv) {
    int optind = 1;
    while (optind < argc && argv[optind][0] == '-') {
        switch (argv[optind][1]) {
            case 'n':
                monitor_print_non_stop();
                break;

            case 'h':
            default:
                monitor_print_help();
                break;
        }
        optind++;
        argv += optind;
    }
    if (argc == 1) {
        monitor_print_all();
    } else if (argc > 1 && optind == 1) {
        monitor_print_help();
    }
    return 0;
}
