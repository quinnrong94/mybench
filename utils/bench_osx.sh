#!/bin/bash

set -x

function run() {
    sys=$1

    rm -rf build${sys}
    mkdir build${sys}

    pushd build${sys}
    cmake ..
    make
    popd

    ./build${sys}/cpu_test > tmp.txt 2>&1
    cat tmp.txt
    cat tmp.txt >> log_${sys}.txt
    rm tmp.txt
}

run 64
