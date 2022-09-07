#!/bin/bash

set -ex

nvcc -std=c++11                                 \
    --generate-code arch=compute_75,code=sm_75  \
    main-cuda.cpp                               \
    mix_kernels_cuda.cu                         \
    -o bench

./bench
