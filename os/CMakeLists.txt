cmake_minimum_required(VERSION 3.6)

project(os C ASM)

if (OS STREQUAL "freertos")
    list(APPEND OS_INC
        freertos
        freertos/cfg
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
        freertos/services/cat
        freertos/services/cli
        freertos/services/file
        freertos/services/misc
        freertos/services/monitor
        freertos/services/serial_bridge
        freertos/services/service
        freertos/services/ls
        freertos/services/cd
        freertos/services/pwd
        freertos/services/rm
    )

    list(APPEND OS_SRC
        freertos/core/src/croutine.c
        freertos/core/src/hooks.c
        freertos/core/src/list.c
        freertos/core/src/queue.c
        freertos/core/src/tasks.c
        freertos/core/src/timers.c
        freertos/memmang/heap_custom.c
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
        freertos/services/misc/bit.c
        freertos/services/misc/log.c
        freertos/services/misc/ring_buf.c
        freertos/services/cat/cat.c
        freertos/services/cli/cmd.c
        freertos/services/cli/node.c
        freertos/services/cli/service.c
        freertos/services/file/file.c
        freertos/services/monitor/monitor.c
        freertos/services/serial_bridge/serial_bridge.c
        freertos/services/service/service.c
        freertos/services/ls/ls.c
        freertos/services/cd/cd.c
        freertos/services/pwd/pwd.c
        freertos/services/rm/rm.c
    )
elseif (OS STREQUAL "posix")
    # OS specific sources
elseif (OS STREQUAL "qnx")
    list(APPEND OS_SRC
        qnx/string.c
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
        fatfs/fsa/null.c
        fatfs/fsa/printf.c
        fatfs/fsa/stdio.c
        fatfs/fsa/dirent.c
        fatfs/port/diskio.c
    )
elseif (FS STREQUAL "posix")
    # FS specific sources
elseif (FS STREQUAL "qnx")
    # FS specific sources
else ()
    message(FATAL_ERROR "File system is not selected!")
endif ()

add_library(os-static STATIC ${OS_SRC})
target_include_directories(os-static PUBLIC ${OS_INC})

list(TRANSFORM OS_INC PREPEND "catpilot/os/")
set(OS_EXT_INC ${OS_INC} PARENT_SCOPE)
