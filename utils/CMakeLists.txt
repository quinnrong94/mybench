cmake_minimum_required(VERSION 3.1)

project(cpu)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIE -pie")

add_executable(cpu_test main.cc cpu_utils.cc cpu_info.cc)

