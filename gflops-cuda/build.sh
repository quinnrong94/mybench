#!/bin/bash

set -ex

FP16=${1:-0}
if [ $FP16 == 0 ]; then
    nvcc                                            \
        --generate-code arch=compute_75,code=sm_75  \
        bench_fp32.cu                               \
        -o bench_fp32
    ./bench_fp32
else
    nvcc                                            \
        --generate-code arch=compute_75,code=sm_75  \
        bench_fp16.cu                               \
        -o bench_fp16
    ./bench_fp16
fi
