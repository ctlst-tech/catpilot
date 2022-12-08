#!/bin/bash

BINARY_PATH="./build/firmware/ctlst-fmuv5.elf"

if [ -n "$1" ]
then
    BINARY_PATH=$1
    echo $BINARY_PATH
fi

openocd -f interface/stlink.cfg -fj ./bsp/mcu/core/stm32/h753/stm32h753.cfg -c "init" -c "program $BINARY_PATH verify reset exit"
