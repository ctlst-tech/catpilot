cmake_minimum_required(VERSION 3.15)

add_subdirectory(c-atom)

if (BOARD STREQUAL "cube")
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
    add_subdirectory(bsp)
    add_subdirectory(os)
    target_link_libraries(c-atom-static PUBLIC os-static)
    target_link_libraries(eswb-static PUBLIC os-static)
elseif (BOARD STREQUAL "ctlst")
    add_subdirectory(os)
    target_link_libraries(c-atom-static PUBLIC os-static)
    target_link_libraries(eswb-static PUBLIC os-static)
endif()

set(OS ${OS_EXT_INC} PARENT_SCOPE)
set(BSP ${BSP_EXT_INC} PARENT_SCOPE)
