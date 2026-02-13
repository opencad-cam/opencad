@echo off
setlocal
echo === OpenCAD Packaging Script ===

:: Set Paths (Adjust these if your environment differs)
set MINGW_PATH=C:\Qt\Tools\mingw1310_64\bin
set QT_PATH=C:\Qt\6.10.1\mingw_64
set CMAKE_PATH=C:\Qt\Tools\CMake_64\bin
set NINJA_PATH=C:\Qt\Tools\Ninja
set VCPKG_ROOT=C:\vcpkg

:: Add to PATH
set PATH=%CMAKE_PATH%;%NINJA_PATH%;%MINGW_PATH%;%QT_PATH%\bin;%PATH%

:: Set Qt6 directory
set Qt6_DIR=%QT_PATH%\lib\cmake\Qt6

:: Output directories
set BUILD_DIR=build
set PKG_DIR=packages

if not exist %PKG_DIR% mkdir %PKG_DIR%

:: Configure
echo.
echo [1/3] Configuring...
cmake -B %BUILD_DIR% -G Ninja ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake ^
    -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic ^
    -DVCPKG_HOST_TRIPLET=x64-mingw-dynamic ^
    -DQt6_DIR=%Qt6_DIR% ^
    -DCMAKE_PREFIX_PATH=%QT_PATH% ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_C_COMPILER=%MINGW_PATH%/gcc.exe ^
    -DCMAKE_CXX_COMPILER=%MINGW_PATH%/g++.exe ^
    -DCMAKE_MAKE_PROGRAM=%NINJA_PATH%/ninja.exe

if %ERRORLEVEL% NEQ 0 goto :error

:: Build
echo.
echo [2/3] Building...
ninja -C %BUILD_DIR%

if %ERRORLEVEL% NEQ 0 goto :error

:: Package
echo.
echo [3/3] Packaging...
cd %BUILD_DIR%
cpack -G NSIS
if %ERRORLEVEL% NEQ 0 (
    echo NSIS packaging failed or NSIS not installed. Trying ZIP...
    cpack -G ZIP
)
cd ..

:: Copy artifacts
echo.
echo Copying packages to %PKG_DIR%...
move %BUILD_DIR%\OpenCAD-*.exe %PKG_DIR%\
move %BUILD_DIR%\OpenCAD-*.zip %PKG_DIR%\

echo.
echo === Packaging Successful! ===
echo Check the %PKG_DIR% directory.
goto :eof

:error
echo.
echo !!! Packaging Failed !!!
pause
exit /b 1
