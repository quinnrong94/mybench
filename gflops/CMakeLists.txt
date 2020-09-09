cmake_minimum_required(VERSION 3.1)

project(bench)

enable_language(ASM)

add_executable(bench_gflops bench.cc FMLA.S)
