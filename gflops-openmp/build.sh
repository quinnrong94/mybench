#!/bin/bash

clang++ bench.cc FMLA.S \
    -o bench \
    -fopenmp \
    -L/opt/homebrew/opt/libomp/lib
