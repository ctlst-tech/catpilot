cmake_minimum_required(VERSION 3.15)

set(PROJ_NAME "catpilot")

set(ASM_FILE ${PROJECT_BINARY_DIR}/${PROJ_NAME}.asm)
# TODO: Use defines from bsp-static lib
set(CMAKE_TOOLCHAIN_FILE bsp/mcu/core/stm32/h753/stm32h753_toolchain.cmake)

# TODO: Replace to OS CMakeLists
include_directories(BEFORE
    osa/freertos_posix/include/FreeRTOS_POSIX
    osa/freertos_posix/include
    osa/freertos_posix/include
    osa/freertos_posix/FreeRTOS-Plus-POSIX/include
    osa/freertos_posix/FreeRTOS-Plus-POSIX/include/portable/
    osa/freertos_posix/include/private
    os/freertos/core/inc
    os/freertos/
    os/freertos/port/cortex_m7/
)

set(MAP_CREATION "ON")
set(ESWB_EQRB_NO_SOCKET "1")
set(CATOM_NO_SOCKET "1")
set(FAKE_PTHREAD "1")

project(${PROJ_NAME} C ASM)

set(ARCH cortex_m7)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/firmware)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "" )
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "" )

# TODO: Replace to OS CMakeLists
set(DIR_OS os/freertos)
set(DIR_FS os/fatfs)
set(DIR_OSA osa)

set(DIR_C_ATOM c-atom)
set(DIR_ESWB c-atom/eswb)

# TODO: Replace to lib CMakeLists
set(DIR_MDL modules)
set(DIR_MISC misc)

include_directories(
    ${DIR_OS}
    ${DIR_OS}/core/inc
    ${DIR_OS}/port/${ARCH}

    ${DIR_C_ATOM}/swsys

    ${DIR_FS}

    ${DIR_OSA}/fs_posix

    ${DIR_MDL}
    ${DIR_MDL}/common
    ${DIR_MDL}/logger

    ${DIR_MISC}/common
)

add_subdirectory(bsp)
add_subdirectory(c-atom)
include(fspecs.cmake)

set(SRC_MAIN main.c)
set(XML_INLINE xml_inline.c xml_inline_cfgs.c)

file(GLOB_RECURSE SRC_MDL    ${DIR_MDL}/*.c)
file(GLOB_RECURSE SRC_MISC   ${DIR_MISC}/*.c)
file(GLOB_RECURSE SRC_FS     ${DIR_FS}/*.c)
file(GLOB_RECURSE SRC_OS     ${DIR_OS}/core/*.c
                             ${DIR_OS}/port/${ARCH}/*.c
                             ${DIR_OS}/MemMang/heap_3.c
                             ${DIR_OS}/MemMang/heap.c)
file(GLOB_RECURSE SRC_OSA    ${DIR_OSA}/*.c)

set(SOURCES
    ${SRC_MAIN}
    ${XML_INLINE}
    ${SRC_MDL}
    ${SRC_MISC}
    ${SRC_FS}
    ${SRC_OS}
    ${SRC_OSA}
    ${FSPECS_REG}
)

add_executable(${PROJECT_NAME}.elf
    ${DIR_C_ATOM}/function/error.c # FIXME
    ${DIR_C_ATOM}/function/conv.c # FIXME
)

target_link_libraries(${PROJECT_NAME}.elf PRIVATE
    bsp-static
    c-atom-static
    ${FSPECS_LIBS}
    -lc -lm -lnosys
)

set(ELF_FILE ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.elf)
set(HEX_FILE ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.hex)
set(BIN_FILE ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.bin)

add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -Obinary ${ELF_FILE} ${BIN_FILE}
    COMMAND ${CMAKE_OBJCOPY} -Oihex  ${ELF_FILE} ${HEX_FILE}
)
add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${HEX_FILE} "${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.hex"
    COMMAND ${CMAKE_COMMAND} -E copy ${BIN_FILE} "${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.bin"
    COMMAND ${CMAKE_SIZE} -A ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.elf
    COMMAND ${CMAKE_SIZE} --format=berkeley ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.hex
)
