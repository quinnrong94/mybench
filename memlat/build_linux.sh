#!/bin/bash

export VERBOSE=1

rm -rf build
mkdir build

pushd build
cmake ..
make
popd
