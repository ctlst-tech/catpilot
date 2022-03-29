cmake_minimum_required(VERSION 3.15)

set(PROJ_NAME "ctlst-fmuv5")

set(CMAKE_VERBOSE_MAKEFILE OFF)
if (TYPE STREQUAL "PX4")
    set(MAP_CREATION "ON")
    set(MCU stm32f765ii)
    set(DIR_MCU board/mcu/${MCU})
    set(ASM_FILE ${PROJECT_BINARY_DIR}/${PROJ_NAME}.asm)
    set(CMAKE_TOOLCHAIN_FILE ${DIR_MCU}/core/f7_toolchain.cmake)
elseif (TYPE STREQUAL "Posix")
    set(DIR_POSIX board/posix)
    set(CMAKE_TOOLCHAIN_FILE ${DIR_POSIX}/posix_toolchain.cmake)
else ()
    message(FATAL_ERROR "TYPE is not set!")
endif ()

project(${PROJ_NAME} C ASM)

if (TYPE STREQUAL "PX4")

    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/px4)
    set(CMAKE_BINARY_DIR ./build/px4)

    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "" )
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "" )

    set(DIR_OS os/freertos)
    set(DIR_OSA osa/)
    set(DIR_FS os/fatfs)

    set(DIR_PERIPH ${DIR_MCU}/periph)
    set(DIR_CONF ${DIR_MCU}/conf)
    set(DIR_DRV ${DIR_MCU}/drv)

    # Temporarily
    set(DIR_MDL modules)
    set(DIR_LIB lib)

    set(HEAP_SIZE 0x400)

    add_definitions(
        -DSTM32
        -DSTM32F7
        -DHEAP_SIZE=${HEAP_SIZE}
    )

    include_directories(
        ${DIR_MCU}
        ${DIR_MCU}/core
        ${DIR_MCU}/hal/inc

        ${DIR_OS}/core/inc
        ${DIR_OS}/port/${MCU}
        ${DIR_OSA}
        ${DIR_FS}

        ${DIR_PERIPH}
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

        ${DIR_DRV}
        ${DIR_DRV}/icm20602
        ${DIR_DRV}/ist8310
        ${DIR_DRV}/sdcard
        ${DIR_DRV}/px4io

        ${DIR_MDL}/cli
        ${DIR_MDL}/sensors
        ${DIR_MDL}/logger
        ${DIR_MDL}/io

        ${DIR_LIB}/

        ${DIR_CONF}
        ${CMAKE_CURRENT_BINARY_DIR}
    )

    set(SRC_MAIN main.c)
    file(GLOB STARTUP            ${DIR_MCU}/core/*.s)
    file(GLOB_RECURSE SRC_MCU    ${DIR_MCU}/*.c)
    file(GLOB_RECURSE SRC_PERIPH ${DIR_PERIPH}/*.c)
    file(GLOB_RECURSE SRC_DRV    ${DIR_DRV}/*.c)
    file(GLOB_RECURSE SRC_MDL    ${DIR_MDL}/*.c)
    file(GLOB_RECURSE SRC_LIB    ${DIR_LIB}/*.c)
    file(GLOB_RECURSE SRC_FS     ${DIR_FS}/*.c)
    file(GLOB_RECURSE SRC_OS     ${DIR_OS}/core/*.c ${DIR_OS}/port/${MCU}/*.c ${DIR_OS}/MemMang/heap_1.c)

    set(SOURCES
        ${SRC_MAIN}
        ${STARTUP}
        ${SRC_MCU}
        ${SRC_PERIPH}
        ${SRC_DRV}
        ${SRC_MDL}
        ${SRC_LIB}
        ${SRC_OS}
        ${SRC_FS}
    )

    add_executable(${PROJECT_NAME}.elf ${SOURCES})
    target_link_libraries(${PROJECT_NAME}.elf -lc -lm -lnosys)

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

elseif (TYPE STREQUAL "Posix")

    find_package(Threads)

    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/posix)
    set(CMAKE_BINARY_DIR build/posix)

    set(DIR_POSIX board/posix)
    set(DIR_OS os/freertos)
    set(DIR_CONF ${DIR_POSIX}/conf)

    include_directories(
        ${DIR_OS}/core/inc
        ${DIR_OS}/port/posix
        ${DIR_OS}/port/posix/utils
        ${DIR_CONF}
        ${CMAKE_CURRENT_BINARY_DIR}
    )

    set(SRC_MAIN ${DIR_POSIX}/main.c)
    file(GLOB_RECURSE SRC_OS ${DIR_OS}/core/*.c ${DIR_OS}/port/posix/*.c ${DIR_OS}/MemMang/heap_1.c)

    set(SOURCES
        ${SRC_MAIN}
        ${SRC_OS}
    )

    add_executable(${PROJECT_NAME} ${SOURCES})

    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

endif ()
