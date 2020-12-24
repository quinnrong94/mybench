#!/bin/bash

set -x

ANDROID_DIR=/data/local/tmp

function run() {
    sys=$1
    ABI=$2

    rm -rf build${sys}
    mkdir build${sys}

    pushd build${sys}
    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_15c/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=${ABI} \
        -DBUILD_FOR_ANDROID_COMMAND=true
    make
    popd

    adb push build${sys}/cpu_test /data/local/tmp/cpu_test_${sys}
    adb shell "cd ${ANDROID_DIR}; ./cpu_test_${sys} > log_${sys}.txt 2>&1"
    adb pull ${ANDROID_DIR}/log_${sys}.txt tmp.txt

    cat tmp.txt

    cat tmp.txt >> log_${sys}.txt
    rm tmp.txt
}

run 64 arm64-v8a
run 32 armeabi-v7a
