#pragma once

/** CMSIS */
#include "stm32f765xx.h"

/** HAL */
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_conf.h"

/** FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "timers.h"
#include "semphr.h"

/** FreeRTOS POSIX implementation */
#include "FreeRTOS_POSIX.h"

/** Log msg **/
#include "log.h"

/** ERRNO */
#include <errno.h>

/** STD */
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
