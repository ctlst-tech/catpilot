cmake_minimum_required(VERSION 3.15)

# FIXME
include_directories(
    os/${OS}/core/inc
    os/${OS}/cfg
    os/${OS}/port
    os/${OS}/osa/FreeRTOS-Plus-POSIX/include
    os/${OS}/osa/include
    os/${OS}/osa/include/FreeRTOS_POSIX
    os/${OS}/osa/include/FreeRTOS_POSIX/sys
    os/${OS}/osa/include/private
    c-atom/swsys
)

add_subdirectory(os)
add_subdirectory(bsp)
add_subdirectory(c-atom)

include_directories(
    ${OS_EXT_INC}
    ${BSP_EXT_INC}
)

target_link_libraries(c-atom-static PUBLIC os-static)
target_link_libraries(eswb-static PUBLIC os-static)
