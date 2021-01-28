#!/bin/bash

flush_zero=${1:-0}

FZ=""
if [[ ${flush_zero} == 1 ]]; then
    FZ="-DFZ=ON"
fi

rm -rf build_denormal
mkdir build_denormal
pushd build_denormal
    cmake .. ${FZ}
    make
popd

rm -rf build_normal
mkdir build_normal
pushd build_normal
    cmake .. -DNORMAL=ON ${FZ}
    make
popd

