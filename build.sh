#!/bin/bash
src_dir="."
build_dir="build"
if ! cmake --fresh -S "$src_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release; then
    echo "Could not configure using CMake"
    exit 1;
fi

if ! cmake --build "$build_dir" --config Release; then
    echo "Could not build."
    exit 1;
fi

#if ! sudo cmake --install "$build_dir" then
#    echo "Could not install."
#    exit 1;
#fi
