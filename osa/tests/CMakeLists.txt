CMAKE_MINIMUM_REQUIRED(VERSION 3.6)
project(tests)
set(CMAKE_CXX_STANDARD 14)

find_package(Catch2 REQUIRED)

include_directories(../fs_posix/node)

set(NODE_SRC
    node_tests.cpp
    ../fs_posix/node/node.c
)

add_executable(node_tests ${NODE_SRC})
target_link_libraries(node_tests PUBLIC Catch2::Catch2WithMain)
