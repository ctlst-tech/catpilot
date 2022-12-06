#ifndef MONITOR_H_
#define MONITOR_H_

#include <stdint.h>
#include "os.h"
#include "cli.h"

#ifdef __cplusplus
extern "C" {
#endif

void monitor_start_timer(void);
uint32_t monitor_get_counter(void);
int monitor_commander(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif  // MONITOR_H
