#!/bin/bash
rm -r build
mkdir build
cd build
mkdir px4
mkdir posix

# PX4 build
cd px4 && cmake ../.. -DTYPE=PX4 -DCMAKE_BUILD_TYPE=Debug-target
make all

# Posix build
cd ../posix && cmake ../.. -DTYPE=Posix -DCMAKE_BUILD_TYPE=Debug-target
make all
