cmake_minimum_required(VERSION 3.15)

set(PROJ_NAME "catpilot")

if (BOARD STREQUAL "cube")
    set(MCU_FAMILY stm32)
    set(MCU_MODEL h753)
    set(IC_CUBEIO "ON")
    set(IC_ICM20602 "ON")
    set(IC_ICM20649 "ON")
    set(IC_ICM20948 "ON")
    set(IC_MS5611 "ON")
    set(IC_SD "ON")
    set(OS "freertos")
    set(FS "fatfs")
else ()
    message(FATAL_ERROR "Board is not selected!")
endif ()

set(CMAKE_TOOLCHAIN_FILE bsp/mcu/core/${MCU_FAMILY}/${MCU_MODEL}/${MCU_MODEL}_toolchain.cmake)
set(ASM_FILE ${PROJECT_BINARY_DIR}/${PROJ_NAME}.asm)

set(MAP_CREATION "ON")
set(ESWB_EQRB_NO_SOCKET "1")
set(CATOM_NO_SOCKET "1")
set(FAKE_PTHREAD "1")

project(${PROJ_NAME} C ASM)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/firmware)

# FIXME
include_directories(
    os/freertos/core/inc
    os/freertos/cfg
    os/freertos/port
    os/freertos/osa/FreeRTOS-Plus-POSIX/include
    os/freertos/osa/include
    os/freertos/osa/include/FreeRTOS_POSIX
    os/freertos/osa/include/private
    c-atom/swsys
)

add_subdirectory(os)
add_subdirectory(bsp)
add_subdirectory(c-atom)
include(fspecs.cmake)

add_executable(${PROJECT_NAME}.elf
    main.c
    xml_inline.c
    xml_inline_cfgs.c
    # FIXME
    c-atom/function/error.c
    c-atom/function/conv.c
)

target_link_libraries(${PROJECT_NAME}.elf PUBLIC
    os-static
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
