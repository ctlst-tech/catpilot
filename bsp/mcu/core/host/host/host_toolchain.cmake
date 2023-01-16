cmake_minimum_required(VERSION 3.6)

set(TOOLCHAIN HOST)
set(HOST TRUE)

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_ASM_COMPILER g++)
set(SIZE size)
set(CMAKE_OBJCOPY objcopy)
set(CMAKE_OBJDUMP objdump)

set(COMMON_FLAGS "-Wall -Wextra")

if (CMAKE_BUILD_TYPE STREQUAL "Debug-target")
    add_definitions(-DDEBUG)
    set(CMAKE_C_FLAGS_INIT "${COMMON_FLAGS} -O0 -g3")
    set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} -O0 -g3")
    set(CMAKE_EXE_LINKER_FLAGS_INIT "${COMMON_FLAGS} -O0 -g3")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release-target")
    set(CMAKE_C_FLAGS_INIT "${COMMON_FLAGS} -Os -g0")
    set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} -Os -g0")
    set(CMAKE_EXE_LINKER_FLAGS_INIT "${COMMON_FLAGS} -Os -g0")
endif ()

