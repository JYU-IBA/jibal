#!/bin/bash
mkdir build
cd build
rm CMakeCache.txt
cmake ../
make
#make install
