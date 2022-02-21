set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_SYSTEM_VERSION 1)

if (MAP_CREATION STREQUAL "ON")
    set(LINK_MAP_CREATION_FLAG "-Wl,-Map=${PROJECT_BINARY_DIR}/${PROJ_NAME}.map")
endif ()

set(ARCH "cortex-m7")

SET(CMAKE_C_COMPILER_WORKS   1)
SET(CMAKE_CXX_COMPILER_WORKS 1)

set(TOOLCHAIN_BIN_DIR   "/usr/bin")
set(TOOLCHAIN_BIN_PATH "${TOOLCHAIN_BIN_DIR}/arm-none-eabi")
set(CMAKE_C_COMPILER    ${TOOLCHAIN_BIN_PATH}-gcc)
set(CMAKE_CXX_COMPILER  ${TOOLCHAIN_BIN_PATH}-g++)
set(CMAKE_ASM_COMPILER  ${TOOLCHAIN_BIN_PATH}-gcc)
set(CMAKE_SIZE          ${TOOLCHAIN_BIN_PATH}-size)
set(CMAKE_OBJCOPY       ${TOOLCHAIN_BIN_PATH}-objcopy)
set(CMAKE_OBJDUMP       ${TOOLCHAIN_BIN_PATH}-objdump)

file(GLOB LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/mcu/stm32f765ii/core/*.ld)

set(MCU_FLAGS "-mcpu=cortex-m7 -mfloat-abi=hard")

set(COMMON_FLAGS "${MCU_FLAGS} -Wall -Wextra -nostartfiles -nodefaultlibs -nostdlib -fdata-sections -ffunction-sections -Wl,--gc-sections")
set(LINKER_FLAGS "${LINK_MAP_CREATION_FLAG} ${COMMON_FLAGS} -Wl,--start-group -lgcc -lc -lg -Wl,--end-group -Wl,--gc-sections -u _printf_float -T ${LINKER_SCRIPT}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug-target")
    add_definitions(-DDEBUG)
    set(CMAKE_C_FLAGS_INIT "${COMMON_FLAGS} -O0 -g0")
    set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} -O0 -g0")
    set(CMAKE_EXE_LINKER_FLAGS_INIT "${COMMON_FLAGS} -O0 -g0 ${LINKER_FLAGS}")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release-target")
    set(CMAKE_C_FLAGS_INIT "${COMMON_FLAGS} -Os -g0")
    set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} -Os -g0")
    set(CMAKE_EXE_LINKER_FLAGS_INIT "${COMMON_FLAGS} -Os -g0 ${LINKER_FLAGS}")
endif ()
