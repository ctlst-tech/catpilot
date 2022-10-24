CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

set(TOOLCHAIN_BIN_PATH "/usr/bin")
set(CMAKE_C_COMPILER    ${TOOLCHAIN_BIN_PATH}/gcc)
set(CMAKE_CXX_COMPILER  ${TOOLCHAIN_BIN_PATH}/g++)
set(CMAKE_ASM_COMPILER  ${TOOLCHAIN_BIN_PATH}/gcc)
set(CMAKE_SIZE          ${TOOLCHAIN_BIN_PATH}/size)
set(CMAKE_OBJCOPY       ${TOOLCHAIN_BIN_PATH}/objcopy)
set(CMAKE_OBJDUMP       ${TOOLCHAIN_BIN_PATH}/objdump)
set(CMAKE_CXX_STANDARD 14)

project(tests)

find_package(Catch2 REQUIRED)

include_directories(../fs_posix/node)

file(GLOB NODE_SRC
    "node_tests.cpp"
    "../fs_posix/node/node.c"
)

add_executable(node_tests ${NODE_SRC})
target_link_libraries(node_tests PUBLIC Catch2::Catch2WithMain)
