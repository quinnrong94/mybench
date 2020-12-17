#!/bin/bash

set -x

ANDROID_DIR=/data/local/tmp

rm -rf build64
mkdir build64

pushd build64
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI="arm64-v8a"
make
popd

adb push build64/cpu_test /data/local/tmp
# adb shell "cat /proc/cpuinfo > ${ANDROID_DIR}/log.txt"
adb shell "cd ${ANDROID_DIR}; ./cpu_test >> log.txt 2>&1"
adb pull ${ANDROID_DIR}/log.txt tmp.txt

cat tmp.txt

cat tmp.txt >> log.txt
rm tmp.txt
