@echo off
setlocal

set BUILD_TYPE=Release

if "%1" == "1" (
    if exist build (
        rmdir /s /q build
    )
)

if not exist build (
    mkdir build
)

pushd build
    cmake ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        ..
    cmake --build . --config %BUILD_TYPE%
popd

.\build\%BUILD_TYPE%\bench.exe
