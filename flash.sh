#!/bin/bash

BINARY_PATH="build/firmware/ctlst-fmuv5.bin"

if [ -n "$1" ]
then
    BINARY_PATH=$1
    echo $BINARY_PATH
fi

st-flash write $BINARY_PATH 0x08000000
