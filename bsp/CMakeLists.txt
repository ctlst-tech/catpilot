cmake_minimum_required(VERSION 3.6)

project(bsp C ASM)

list(APPEND BSP_INC
    mcu/core/${MCU_FAMILY}/common
    mcu/core/${MCU_FAMILY}/${MCU_MODEL}
    mcu/core/${MCU_FAMILY}/${MCU_MODEL}/common
    mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal
    mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/inc
    boards/${BOARD}
    boards/${BOARD}/cfg
    boards/${BOARD}/startup
)

if (MCU_FAMILY STREQUAL "stm32")
    if(MCU_MODEL STREQUAL "h753")
        list(APPEND BSP_SRC
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/common/system_stm32h7xx.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/common/syscalls.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/startup/startup_stm32h753xx.s
        )
        list(APPEND BSP_SRC
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_adc_ex.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_adc.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_cortex.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_dma.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_dma_ex.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_exti.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_fdcan.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_flash.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_flash_ex.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_gpio.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_hcd.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_i2c.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_i2c_ex.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_pcd.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_pcd_ex.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_pwr_ex.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_rcc.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_rcc_ex.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_sd.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_sd_ex.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_spi.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_spi_ex.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_tim.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_tim_ex.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_uart.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_uart_ex.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_usart.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_hal_usart_ex.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_ll_sdmmc.c
            mcu/core/${MCU_FAMILY}/${MCU_MODEL}/hal/src/stm32h7xx_ll_usb.c
        )
        list(APPEND BSP_SRC
            boards/${BOARD}/cfg/adc_cfg.c
            boards/${BOARD}/cfg/can_cfg.c
            boards/${BOARD}/cfg/exti_cfg.c
            boards/${BOARD}/cfg/gpio_cfg.c
            boards/${BOARD}/cfg/i2c_cfg.c
            boards/${BOARD}/cfg/ics_cfg.c
            boards/${BOARD}/cfg/sdio_cfg.c
            boards/${BOARD}/cfg/spi_cfg.c
            boards/${BOARD}/cfg/tim_cfg.c
            boards/${BOARD}/cfg/usart_cfg.c
            boards/${BOARD}/cfg/usb_cfg.c
            boards/${BOARD}/cfg/cli_cfg.c
            boards/${BOARD}/startup/init.c
            boards/${BOARD}/external.c
        )
    endif()

    list(APPEND BSP_SRC
        mcu/periph/adc/${MCU_FAMILY}/adc.c
        mcu/periph/can/${MCU_FAMILY}/can.c
        mcu/periph/dma/${MCU_FAMILY}/dma.c
        mcu/periph/exti/${MCU_FAMILY}/exti.c
        mcu/periph/gpio/${MCU_FAMILY}/gpio.c
        mcu/periph/i2c/${MCU_FAMILY}/i2c.c
        mcu/periph/irq/${MCU_FAMILY}/irq.c
        mcu/periph/sdio/${MCU_FAMILY}/sdio.c
        mcu/periph/spi/${MCU_FAMILY}/spi.c
        mcu/periph/tim/${MCU_FAMILY}/tim.c
        mcu/periph/usart/${MCU_FAMILY}/usart.c
        mcu/periph/usb/${MCU_FAMILY}/usb.c
        mcu/periph/usb/${MCU_FAMILY}/USB_DEVICE/Target/usbd_conf.c
        mcu/periph/usb/${MCU_FAMILY}/USB_DEVICE/App/usb_device.c
        mcu/periph/usb/${MCU_FAMILY}/USB_DEVICE/App/usbd_cdc_if.c
        mcu/periph/usb/${MCU_FAMILY}/USB_DEVICE/App/usbd_desc.c
        mcu/periph/usb/${MCU_FAMILY}/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c
        mcu/periph/usb/${MCU_FAMILY}/STM32_USB_Device_Library/Core/Src/usbd_core.c
        mcu/periph/usb/${MCU_FAMILY}/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
        mcu/periph/usb/${MCU_FAMILY}/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
    )

    list(APPEND BSP_INC
        mcu/periph
        mcu/periph/adc/${MCU_FAMILY}
        mcu/periph/can/${MCU_FAMILY}
        mcu/periph/dma/${MCU_FAMILY}
        mcu/periph/exti/${MCU_FAMILY}
        mcu/periph/gpio/${MCU_FAMILY}
        mcu/periph/i2c/${MCU_FAMILY}
        mcu/periph/irq/${MCU_FAMILY}
        mcu/periph/sdio/${MCU_FAMILY}
        mcu/periph/spi/${MCU_FAMILY}
        mcu/periph/tim/${MCU_FAMILY}
        mcu/periph/usart/${MCU_FAMILY}
        mcu/periph/usb/${MCU_FAMILY}
        mcu/periph/usb/${MCU_FAMILY}/USB_DEVICE/Target
        mcu/periph/usb/${MCU_FAMILY}/USB_DEVICE/App
        mcu/periph/usb/${MCU_FAMILY}/STM32_USB_Device_Library/Class/CDC/Inc
        mcu/periph/usb/${MCU_FAMILY}/STM32_USB_Device_Library/Core/Inc
    )
elseif (MCU_FAMILY STREQUAL "zynq")
    list(APPEND BSP_SRC
        mcu/periph/adc/${MCU_FAMILY}/ad7606b.c
        mcu/periph/gpio/${MCU_FAMILY}/gpio.c
        mcu/periph/spi/${MCU_FAMILY}/spiplr.c
    )
    list(APPEND BSP_INC 
        mcu/periph/adc/${MCU_FAMILY}
        mcu/periph/gpio/${MCU_FAMILY}
        mcu/periph/spi/${MCU_FAMILY}
    )
endif()

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

if (IC_CTLST STREQUAL "ON")
    list(APPEND BSP_SRC ics/ctlst/adxrs450.c)
    list(APPEND BSP_SRC ics/ctlst/ad7190.c)
    list(APPEND BSP_SRC ics/ctlst/ctlst_periph.c)
    list(APPEND BSP_SRC ics/ctlst/ctlst_nav.c)
    list(APPEND BSP_INC ics/ctlst)
endif ()

add_library(bsp-static STATIC ${BSP_SRC})
target_include_directories(bsp-static PUBLIC ${BSP_INC})
target_link_libraries(bsp-static os-static)

list(TRANSFORM BSP_INC PREPEND "catpilot/bsp/")
set(BSP_EXT_INC ${BSP_INC} PARENT_SCOPE)
