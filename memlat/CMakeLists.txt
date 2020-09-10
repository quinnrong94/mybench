cmake_minimum_required(VERSION 3.1)

project(bench)

add_executable(bench_memlat main.c lat_mem.c lib_timing.c)
