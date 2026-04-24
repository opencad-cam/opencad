@echo off
set MINGW_PATH=C:\Qt\Tools\mingw1310_64\bin
set QT_PATH=C:\Qt\6.10.1\mingw_64
set CMAKE_PATH=C:\Qt\Tools\CMake_64\bin
set NINJA_PATH=C:\Qt\Tools\Ninja
set VCPKG_ROOT=C:\vcpkg

set PATH=%CMAKE_PATH%;%NINJA_PATH%;%MINGW_PATH%;%QT_PATH%\bin;%VCPKG_ROOT%\installed\x64-mingw-dynamic\bin;%PATH%
set PATH=%~dp0\build\bin;%PATH%

echo Running benchmark...
build\bin\test_benchmark_solver.exe
if %ERRORLEVEL% NEQ 0 (
    echo Benchmark failed with error code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)
echo Benchmark finished.
