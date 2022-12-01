CMAKE_MINIMUM_REQUIRED(VERSION 3.6)
project(fs-tests CXX C)
set(CMAKE_CXX_STANDARD 14)

find_package(Catch2 REQUIRED)

set(NODE_SRC
    node.cpp
    ../fsa/node.h
    ../fsa/node.c
)

add_executable(node ${NODE_SRC})
target_link_libraries(node PUBLIC Catch2::Catch2WithMain)
