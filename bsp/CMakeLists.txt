cmake_minimum_required(VERSION 3.6)
project(bsp)

if (BOARD STREQUAL "cube")
    set(MCU_FAMILY stm32)
    set(MCU_MODEL h753)
    set(MCU_SERIES stm32h753)
    set(IC_CUBEIO "ON")
    set(IC_ICM20602 "ON")
    set(IC_ICM20649 "ON")
    set(IC_ICM20948 "ON")
    set(IC_MS5611 "ON")
    set(IC_SD "ON")
else ()
    message(FATAL_ERROR "BOARD is not selected!")
endif ()

list(APPEND BSP_INC
    mcu/core/${MCU_FAMILY}/common
    mcu/core/${MCU_FAMILY}/${MCU_MODEL}
    mcu/core/${MCU_FAMILY}/${MCU_MODEL}/common
    mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal
    mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/inc
)

file(GLOB_RECURSE BSP_SRC
    mcu/core/${MCU_FAMILY}/${MCU_MODEL}/common/*.c
    mcu/core/${MCU_FAMILY}/${MCU_MODEL}/startup/*.s
    mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/*.c
)

list(APPEND BSP_SRC
    mcu/periph/adc/${MCU_FAMILY}/adc.c
    mcu/periph/dma/${MCU_FAMILY}/dma.c
    mcu/periph/exti/${MCU_FAMILY}/exti.c
    mcu/periph/gpio/${MCU_FAMILY}/gpio.c
    mcu/periph/i2c/${MCU_FAMILY}/i2c.c
    mcu/periph/sdio/${MCU_FAMILY}/sdio.c
    mcu/periph/spi/${MCU_FAMILY}/spi.c
    mcu/periph/tim/${MCU_FAMILY}/tim.c
    mcu/periph/usart/${MCU_FAMILY}/usart.c
    mcu/periph/usb/${MCU_FAMILY}/usb.c
)

list(APPEND BSP_INC
    mcu/periph
    mcu/periph/adc/${MCU_FAMILY}
    mcu/periph/dma/${MCU_FAMILY}
    mcu/periph/exti/${MCU_FAMILY}
    mcu/periph/gpio/${MCU_FAMILY}
    mcu/periph/i2c/${MCU_FAMILY}
    mcu/periph/sdio/${MCU_FAMILY}
    mcu/periph/spi/${MCU_FAMILY}
    mcu/periph/tim/${MCU_FAMILY}
    mcu/periph/usart/${MCU_FAMILY}
    mcu/periph/usb/${MCU_FAMILY}
)

if (IC_CUBEIO STREQUAL "ON")
    list(APPEND BSP_SRC ics/cubeio/cubeio.c)
    list(APPEND BSP_INC ics/cubeio)
endif ()

if (IC_ICM20602 STREQUAL "ON")
    list(APPEND BSP_SRC ics/icm20602/icm20602.c)
    list(APPEND BSP_INC ics/icm20602)
endif ()

if (IC_ICM20649 STREQUAL "ON")
    list(APPEND BSP_SRC ics/icm20649/icm20649.c)
    list(APPEND BSP_INC ics/icm20649)
endif ()

if (IC_ICM20689 STREQUAL "ON")
    list(APPEND BSP_SRC ics/icm20689/icm20689.c)
    list(APPEND BSP_INC ics/icm20689)
endif ()

if (IC_ICM20948 STREQUAL "ON")
    list(APPEND BSP_SRC ics/icm20948/icm20948.c)
    list(APPEND BSP_INC ics/icm20948)
endif ()

if (IC_IST8310 STREQUAL "ON")
    list(APPEND BSP_SRC ics/ist8310/ist8310.c)
    list(APPEND BSP_INC ics/ist8310)
endif ()

if (IC_MS5611 STREQUAL "ON")
    list(APPEND BSP_SRC ics/ms5611/ms5611.c)
    list(APPEND BSP_INC ics/ms5611)
endif ()

if (IC_PX4IO STREQUAL "ON")
    list(APPEND BSP_SRC ics/px4io/px4io.c)
    list(APPEND BSP_INC ics/px4io)
endif ()

if (IC_SD STREQUAL "ON")
    list(APPEND BSP_SRC ics/sd/sd.c)
    list(APPEND BSP_INC ics/sd)
endif ()

list(APPEND BSP_INC 
    boards/${BOARD}
    boards/${BOARD}/cfg
    boards/${BOARD}/startup
)
file(GLOB_RECURSE BSP_SRC 
    boards/${BOARD}/cfg/*.c
    boards/${BOARD}/startup/*.c
)

add_library(bsp-static STATIC ${BSP_SRC})
target_include_directories(bsp-static PUBLIC ${BSP_INC})
