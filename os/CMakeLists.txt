cmake_minimum_required(VERSION 3.6)

project(os C ASM)

if (OS STREQUAL "freertos")
    list(APPEND OS_INC
        freertos
        freertos/cfg
        freertos/cli
        freertos/core/inc
        freertos/inc
        freertos/osa
        freertos/port
        freertos/osa/include
        freertos/osa/include/FreeRTOS_POSIX
        freertos/osa/include/FreeRTOS_POSIX/sys
        freertos/osa/include/FreeRTOS_POSIX/netinet
        freertos/osa/include/FreeRTOS_POSIX/arpa
        freertos/osa/include/private
        freertos/osa/FreeRTOS-Plus-POSIX/include
        freertos/osa/FreeRTOS-Plus-POSIX/include/portable
        freertos/usr/cli
        freertos/usr/misc
        freertos/usr/monitor
        freertos/usr/service
    )

    list(APPEND OS_SRC
        freertos/core/src/croutine.c
        freertos/core/src/hooks.c
        freertos/core/src/list.c
        freertos/core/src/queue.c
        freertos/core/src/tasks.c
        freertos/core/src/timers.c
        freertos/memmang/heap_3.c
        freertos/memmang/heap.c
        freertos/osa/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_clock.c
        freertos/osa/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_mqueue.c
        freertos/osa/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_pthread_barrier.c
        freertos/osa/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_pthread_cond.c
        freertos/osa/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_pthread_mutex.c
        freertos/osa/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_pthread.c
        freertos/osa/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_sched.c
        freertos/osa/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_semaphore.c
        freertos/osa/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_termios.c
        freertos/osa/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_timer.c
        freertos/osa/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_unistd.c
        freertos/osa/FreeRTOS-Plus-POSIX/source/FreeRTOS_POSIX_utils.c
        freertos/port/port.c
        freertos/usr/misc/bit.c
        freertos/usr/misc/log.c
        freertos/usr/misc/ring_buf.c
        freertos/usr/cli/cmd.c
        freertos/usr/cli/node.c
        freertos/usr/cli/service.c
        freertos/usr/monitor/monitor.c
        freertos/usr/service/service.c
    )

else ()
    message(FATAL_ERROR "Operation system is not selected!")
endif ()

if (FS STREQUAL "fatfs")
    list(APPEND OS_INC
        fatfs/core
        fatfs/fsa
        fatfs/port
    )

    list(APPEND OS_SRC
        fatfs/core/ff.c
        fatfs/core/ffsystem.c
        fatfs/core/ffunicode.c
        fatfs/fsa/fatfs.c
        fatfs/fsa/node.c
        fatfs/fsa/printf.c
        fatfs/fsa/stdio.c
        fatfs/port/diskio.c
    )
else ()
    message(FATAL_ERROR "File system is not selected!")
endif ()

add_library(os-static STATIC ${OS_SRC})
target_include_directories(os-static PUBLIC ${OS_INC})

list(TRANSFORM OS_INC PREPEND "catpilot/os/")
set(OS_EXT_INC ${OS_INC} PARENT_SCOPE)
