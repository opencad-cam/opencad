@echo off
echo === OpenCAD Build Script (MinGW + Ninja) ===

:: Set MinGW and Qt paths
set MINGW_PATH=C:\Qt\Tools\mingw1310_64\bin
set QT_PATH=C:\Qt\6.10.1\mingw_64
set CMAKE_PATH=C:\Qt\Tools\CMake_64\bin
set NINJA_PATH=C:\Qt\Tools\Ninja
set VCPKG_ROOT=C:\vcpkg

:: Add to PATH
set PATH=%CMAKE_PATH%;%NINJA_PATH%;%MINGW_PATH%;%PATH%

:: Set Qt6 directory
set Qt6_DIR=%QT_PATH%\lib\cmake\Qt6

:: Clean previous build
if exist build rmdir /s /q build

:: Configure with CMake using Ninja
echo.
echo === Configuring with CMake (Ninja) ===
cmake -B build -G Ninja ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake ^
    -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic ^
    -DVCPKG_HOST_TRIPLET=x64-mingw-dynamic ^
    -DQt6_DIR=%Qt6_DIR% ^
    -DCMAKE_PREFIX_PATH=%QT_PATH% ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_C_COMPILER=%MINGW_PATH%/gcc.exe ^
    -DCMAKE_CXX_COMPILER=%MINGW_PATH%/g++.exe ^
    -DCMAKE_MAKE_PROGRAM=%NINJA_PATH%/ninja.exe

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

:: Build
echo.
echo === Building ===
ninja -C build

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo === Build Successful! ===
echo Binary: build\bin\opencad.exe
pause
