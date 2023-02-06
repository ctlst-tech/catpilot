#ifndef SERIAL_BRIDGE_H_
#define SERIAL_BRIDGE_H_

#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include <termios.h>

#include "cli.h"
#include "os.h"

#ifdef __cplusplus
extern "C" {
#endif

int serial_bridge_start(int priority, int buf_size);
int serial_bridge_commander(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif  // SERIAL_BRIDGE_H_
