CMAKE_MINIMUM_REQUIRED(VERSION 3.6)
project(tests)
set(CMAKE_CXX_STANDARD 14)

find_package(Catch2 REQUIRED)

include(../fsa/node/node.h)

set(NODE_SRC
    node_tests.cpp
    ../fsa/node/node.c
)

add_executable(node_tests ${NODE_SRC})
target_link_libraries(node_tests PUBLIC Catch2::Catch2WithMain)
