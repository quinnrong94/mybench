project(denormal)

cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)

if(NORMAL)
    add_definitions(-DNORMAL)
endif()

if(FZ)
    add_definitions(-DFZ)
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^x86")
    add_definitions(-Dx86)
    add_compile_options(-mavx2 -mavx -mfma)
endif()

add_executable(test_denorm main.cc)
target_compile_options(test_denorm PRIVATE -ffast-math)

