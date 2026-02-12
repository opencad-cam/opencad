@echo off
set MINGW_PATH=C:\Qt\Tools\mingw1310_64\bin
set QT_PATH=C:\Qt\6.10.1\mingw_64\bin
set VCPKG_BIN=C:\vcpkg\installed\x64-mingw-dynamic\bin

:: Add DLL paths to PATH
set PATH=%QT_PATH%;%MINGW_PATH%;%VCPKG_BIN%;%PATH%

echo Starting OpenCAD (logging to opencad.log)...
if exist opencad.log del opencad.log
"build\bin\opencad.exe" > opencad.log 2>&1
