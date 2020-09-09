#!/bin/bash

export VERBOSE=1

rm -rf build32
mkdir build32

pushd build32
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI="armeabi-v7a with NEON"
make
popd

rm -rf build64
mkdir build64

pushd build64
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI="arm64-v8a"
make
popd
