cmake_minimum_required(VERSION 3.1)

project(bench)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/FMLA.h
    COMMAND cmd \/C ${CMAKE_CURRENT_SOURCE_DIR}/compile.bat
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/compile.bat ${CMAKE_CURRENT_SOURCE_DIR}/*.hlsl
    COMMENT "--------------- Compiling DirectX compute shaderes ------------------"
)

add_executable(bench bench.cc FMLA.h)
target_link_libraries(bench d3d11.lib d3dcompiler)
