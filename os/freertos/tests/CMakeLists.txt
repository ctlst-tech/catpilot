CMAKE_MINIMUM_REQUIRED(VERSION 3.6)
project(os-tests CXX C)
set(CMAKE_CXX_STANDARD 14)

find_package(Catch2 REQUIRED)

set(CLI_SRC
    cli.cpp
    ../services/cli/cli.h
    ../services/cli/node.c
)

add_executable(cli ${CLI_SRC})
target_link_libraries(cli PUBLIC Catch2::Catch2WithMain)
