#ifndef CAT_H_
#define CAT_H_

#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include "cli.h"
#include "os.h"

#ifdef __cplusplus
extern "C" {
#endif

int cat_commander(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif  // CAT_H
