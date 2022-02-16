#!/bin/bash

/opt/homebrew/opt/llvm/bin/clang++ bench.cc FMLA.S \
    -o bench \
    -fopenmp \
    -L/opt/homebrew/opt/libomp/lib
