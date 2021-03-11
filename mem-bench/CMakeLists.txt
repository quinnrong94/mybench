cmake_minimum_required(VERSION 3.1)

project(bench)

add_executable(bench_memlat main.c bw_mem.c bw_pipe.c lat_mem.c lib_timing.c)
