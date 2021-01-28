#!/bin/bash

rm -rf build_denormal
mkdir build_denormal
pushd build_denormal
    cmake ..
    make
popd

rm -rf build_normal
mkdir build_normal
pushd build_normal
    cmake .. -DNORMAL=ON
    make
popd
