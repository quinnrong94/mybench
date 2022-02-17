#!/bin/bash

clang++ bench.cc FMLA.S \
    -o bench \
    -Xpreprocessor -fopenmp \
    -I/opt/homebrew/opt/libomp/include \
    -lomp \
    -L/opt/homebrew/opt/libomp/lib
