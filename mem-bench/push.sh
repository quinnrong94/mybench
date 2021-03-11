#!/bin/bash

set -x

export VERBOSE=1

BUILD_PATH=./build32

pushd $BUILD_PATH
make -j6
popd

adb push $BUILD_PATH/bench_memlat /data/local/tmp/test32

BUILD_PATH=./build64

pushd $BUILD_PATH
make -j6
popd

adb push $BUILD_PATH/bench_memlat /data/local/tmp/test64
