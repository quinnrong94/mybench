#!/bin/bash

export VERBOSE=1

normal=${1:-0}

NORMAL=""
if [[ ${normal} == 1 ]]; then
    NORMAL="-DNORMAL=ON"
fi

rm -rf build32
mkdir build32

pushd build32
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI="armeabi-v7a with NEON" ${NORMAL}
make
popd

rm -rf build64
mkdir build64

pushd build64
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI="arm64-v8a" ${NORMAL}
make
popd
