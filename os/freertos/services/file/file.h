#ifndef FILE_H_
#define FILE_H_

#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "cli.h"
#include "os.h"

#ifdef __cplusplus
extern "C" {
#endif

int file_commander(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif  // FILE_H
