cmake_minimum_required(VERSION 3.15)

#-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON

set(PROJ_NAME "ctlst-fmuv5")
set(CMAKE_VERBOSE_MAKEFILE OFF)

if (TYPE STREQUAL "PX4")
    set(MCU stm32f765ii)
    set(BOARD px4)
    set(DIR_MCU bsp/${BOARD}/mcu/${MCU})
    set(ASM_FILE ${PROJECT_BINARY_DIR}/${PROJ_NAME}.asm)
    set(CMAKE_TOOLCHAIN_FILE ${DIR_MCU}/core/f7_toolchain.cmake)
elseif (TYPE STREQUAL "Cube")
    set(MCU stm32h753)
    set(BOARD cube)
    set(DIR_MCU bsp/${BOARD}/mcu/${MCU})
    set(ASM_FILE ${PROJECT_BINARY_DIR}/${PROJ_NAME}.asm)
    set(CMAKE_TOOLCHAIN_FILE ${DIR_MCU}/core/h7_toolchain.cmake)
elseif (TYPE STREQUAL "Linux")
    set(DIR_LINUX bsp/linux)
    set(CMAKE_TOOLCHAIN_FILE ${DIR_LINUX}/linux_toolchain.cmake)
else ()
    message(FATAL_ERROR "TYPE is not set!")
endif ()

if (TYPE STREQUAL "PX4" OR TYPE STREQUAL "Cube")
    include_directories(BEFORE
        osa/freertos_posix/include/FreeRTOS_POSIX
        osa/freertos_posix/include
        osa/freertos_posix/include
        osa/freertos_posix/FreeRTOS-Plus-POSIX/include
        osa/freertos_posix/FreeRTOS-Plus-POSIX/include/portable/
        osa/freertos_posix/include/private
        os/freertos/core/inc
        os/freertos/
        os/freertos/port/stm32f765ii/
    )
    set(MAP_CREATION "ON")
    set(ESWB_EQRB_NO_SOCKET "1")
    set(CATOM_NO_SOCKET "1")
    set(FAKE_PTHREAD "1")
endif ()

project(${PROJ_NAME} C ASM)

if (TYPE STREQUAL "PX4" OR TYPE STREQUAL "Cube")
    set(ARCH cortex_m7)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/firmware)
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "" )
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "" )

    # Defines
    set(DIR_OS os/freertos)
    set(DIR_FS os/fatfs)
    set(DIR_OSA osa)
    set(DIR_PERIPH ${DIR_MCU}/periph)
    set(DIR_IF bsp/${BOARD}/interfaces)
    set(DIR_DEV bsp/${BOARD}/devices)
    set(DIR_DRV drv)
    set(DIR_C_ATOM c-atom)
    set(DIR_ESWB c-atom/eswb)
    set(DIR_MDL modules)
    set(DIR_MISC misc)

    include_directories(
        ${DIR_MCU}
        ${DIR_MCU}/core
        ${DIR_MCU}/hal/inc

        ${DIR_OS}
        ${DIR_OS}/core/inc
        ${DIR_OS}/port/${ARCH}

        ${DIR_C_ATOM}/swsys

        ${DIR_FS}

        ${DIR_OSA}/fs_posix

        ${DIR_PERIPH}
        ${DIR_PERIPH}/adc
        ${DIR_PERIPH}/rcc
        ${DIR_PERIPH}/gpio
        ${DIR_PERIPH}/exti
        ${DIR_PERIPH}/usart
        ${DIR_PERIPH}/spi
        ${DIR_PERIPH}/i2c
        ${DIR_PERIPH}/adc
        ${DIR_PERIPH}/dma
        ${DIR_PERIPH}/hal
        ${DIR_PERIPH}/sdio
        ${DIR_PERIPH}/tim
        ${DIR_PERIPH}/usb

        ${DIR_DRV}
        ${DIR_DRV}/icm20602
        ${DIR_DRV}/icm20689
        ${DIR_DRV}/icm20948
        ${DIR_DRV}/icm20649
        ${DIR_DRV}/ist8310
        ${DIR_DRV}/sdcard
        ${DIR_DRV}/px4io
        ${DIR_DRV}/cli

        ${DIR_IF}
        ${DIR_DEV}

        ${DIR_MDL}
        ${DIR_MDL}/common
        ${DIR_MDL}/logger

        ${DIR_MISC}/common
    )

    add_subdirectory(c-atom)

    include(fspecs.cmake)

    set(SRC_MAIN main.c)
    file(GLOB STARTUP            ${DIR_MCU}/core/*.s)
    file(GLOB_RECURSE SRC_MCU    ${DIR_MCU}/*.c)
    file(GLOB_RECURSE SRC_PERIPH ${DIR_PERIPH}/*.c)
    file(GLOB_RECURSE SRC_DRV    ${DIR_DRV}/*.c)
    file(GLOB_RECURSE SRC_MDL    ${DIR_MDL}/*.c)
    file(GLOB_RECURSE SRC_IF     ${DIR_IF}/*.c)
    file(GLOB_RECURSE SRC_DEV    ${DIR_DEV}/*.c)
    file(GLOB_RECURSE SRC_MISC   ${DIR_MISC}/*.c)
    file(GLOB_RECURSE SRC_FS     ${DIR_FS}/*.c)
    file(GLOB_RECURSE SRC_OS     ${DIR_OS}/core/*.c
                                 ${DIR_OS}/port/${ARCH}/*.c
                                 ${DIR_OS}/MemMang/heap_3.c
                                 ${DIR_OS}/MemMang/heap.c)
    file(GLOB_RECURSE SRC_OSA    ${DIR_OSA}/*.c)

    set(SOURCES
        ${SRC_MAIN}
        ${STARTUP}
        ${SRC_MCU}
        ${SRC_PERIPH}
        ${SRC_DRV}
        ${SRC_MDL}
        ${SRC_IF}
        ${SRC_DEV}
        ${SRC_MISC}
        ${SRC_FS}
        ${SRC_OS}
        ${SRC_OSA}
        ${FSPECS_REG}
    )

    add_executable(${PROJECT_NAME}.elf
        ${SOURCES}
        ${DIR_C_ATOM}/function/error.c # FIXME
        ${DIR_C_ATOM}/function/conv.c # FIXME
    )

    target_link_libraries(${PROJECT_NAME}.elf PRIVATE
        c-atom-static
        ${FSPECS_LIBS}
        -lc -lm -lnosys
    )

    set(ELF_FILE ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.elf)
    set(HEX_FILE ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.hex)
    set(BIN_FILE ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.bin)

    add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Obinary ${ELF_FILE} ${BIN_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Oihex  ${ELF_FILE} ${HEX_FILE}
    )
    add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${HEX_FILE} "${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.hex"
        COMMAND ${CMAKE_COMMAND} -E copy ${BIN_FILE} "${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.bin"
        COMMAND ${CMAKE_SIZE} -A ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.elf
        COMMAND ${CMAKE_SIZE} --format=berkeley ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.hex
    )

elseif (TYPE STREQUAL "Linux")
    find_package(Threads)

    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/linux)
    set(CMAKE_BINARY_DIR build/linux)

    set(DIR_LINUX bsp/linux)
    set(DIR_OS os/freertos)
    set(DIR_CONF ${DIR_LINUX}/conf)

    include_directories(
        ${DIR_OS}/core/inc
        ${DIR_OS}/port/linux
        ${DIR_OS}/port/linux/utils
        ${DIR_CONF}
        ${CMAKE_CURRENT_BINARY_DIR}
    )

    set(SRC_MAIN ${DIR_LINUX}/main.c)
    file(GLOB_RECURSE SRC_OS ${DIR_OS}/core/*.c ${DIR_OS}/port/linux/*.c ${DIR_OS}/MemMang/heap_3.c)

    set(SOURCES
        ${SRC_MAIN}
        ${SRC_OS}
    )

    add_executable(${PROJECT_NAME} ${SOURCES})

    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

endif ()
