cmake_minimum_required(VERSION 2.6...3.13)
project(ctlst-fmuv5 C ASM)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(MCU stm32f765ii)
set(DIR_MCU mcu/${MCU})
set(DIR_PERIPH periph)
set(DIR_DRV drv)
set(DIR_LIB lib)
set(DIR_RTOS freertos)
set(DIR_CONF conf)
set(HEAP_SIZE 0x400)

set(TOOLCHAIN_PATH  "/usr/bin")
SET(CMAKE_C_COMPILER    ${TOOLCHAIN_PATH}/arm-none-eabi-gcc)
SET(CMAKE_CXX_COMPILER  ${TOOLCHAIN_PATH}/arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  ${TOOLCHAIN_PATH}/arm-none-eabi-gcc)
set(CMAKE_AR            ${TOOLCHAIN_PATH}/arm-none-eabi-ar)
set(CMAKE_OBJCOPY       ${TOOLCHAIN_PATH}/arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP       ${TOOLCHAIN_PATH}/arm-none-eabi-objdump)
set(CMAKE_SIZE          ${TOOLCHAIN_PATH}/arm-none-eabi-size)
set(CMAKE_GDB           ${TOOLCHAIN_PATH}/arm-none-eabi-gdb)
set(CMAKE_SIZE          ${TOOLCHAIN_PATH}/arm-none-eabi-size)

add_definitions(
-DSTM32
-DSTM32F7
-DHEAP_SIZE=${HEAP_SIZE}
)

set(CMAKE_BUILD_TYPE "Debug")
set(MCU_FLAGS "-mcpu=cortex-m7 -mlittle-endian -mfloat-abi=hard -mthumb -mno-unaligned-access")
set(CMAKE_C_FLAGS_EXTRA "-nostartfiles -nodefaultlibs -nostdlib -fdata-sections -ffunction-sections")
set(CMAKE_C_FLAGS "${MCU_FLAGS} -Wall -Wextra ${CMAKE_C_FLAGS_EXTRA}")
set(CMAKE_C_DEBUG "-O0 -g3")

# SET(CMAKE_C_FLAGS_INIT "-std=gnu99")

include_directories(
    ${DIR_MCU}/core
    ${DIR_MCU}/hal/inc
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
    ${DIR_DRV}/cli
    ${DIR_DRV}/icm20602
    ${DIR_DRV}/ist8310
    ${DIR_DRV}/sdcard
    ${DIR_LIB}/usr
    ${DIR_LIB}/fatfs
    ${DIR_RTOS}/core/inc
    ${DIR_RTOS}/port/${MCU}
    ${DIR_CONF}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(SRC_MAIN main.c)
file(GLOB STARTUP ${DIR_MCU}/core/*.s)
file(GLOB_RECURSE SRC_MCU ${DIR_MCU}/*.c)
file(GLOB_RECURSE SRC_PERIPH ${DIR_PERIPH}/*.c)
file(GLOB_RECURSE SRC_DRV ${DIR_DRV}/*.c)
file(GLOB_RECURSE SRC_LIB ${DIR_LIB}/*.c)
file(GLOB_RECURSE SRC_RTOS ${DIR_RTOS}/core/*.c ${DIR_RTOS}/port/*.c ${DIR_RTOS}/MemMang/heap_1.c)

set(SOURCES
    ${SRC_MAIN}
    ${STARTUP}
    ${SRC_MCU}
    ${SRC_PERIPH}
    ${SRC_DRV}
    ${SRC_LIB}
    ${SRC_RTOS}
)

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "" )
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "" )

file(GLOB LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/${DIR_MCU}/core/*.ld)
set(CMAKE_EXE_LINKER_FLAGS
"-specs=nano.specs ${MCU_FLAGS} -Wl,--start-group -lgcc -lc -lg -Wl,--end-group -Wl,--gc-sections -u _printf_float -T${LINKER_SCRIPT}"
)

add_executable(${PROJECT_NAME}.elf ${SOURCES})
target_link_libraries(${PROJECT_NAME}.elf -lc -lm -lnosys)

set(ELF_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf)
set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -Obinary ${ELF_FILE} ${BIN_FILE}
    COMMAND ${CMAKE_OBJCOPY} -Oihex  ${ELF_FILE} ${HEX_FILE}
    COMMENT "Building ${PROJECT_NAME}.bin and ${PROJECT_NAME}.hex"

    COMMAND ${CMAKE_COMMAND} -E copy ${HEX_FILE} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex"
    COMMAND ${CMAKE_COMMAND} -E copy ${BIN_FILE} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin"

    COMMAND ${CMAKE_SIZE} --format=berkeley ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMENT "Invoking: Cross ARM GNU Print Size"
)
