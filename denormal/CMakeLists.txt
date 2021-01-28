project(denormal)

cmake_minimum_required(VERSION 3.1)

if(NORMAL)
    add_definitions(-DNORMAL)
endif()

add_executable(test_denorm main.cc)
target_compile_options(test_denorm PRIVATE -ffast-math)
