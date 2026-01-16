@echo off
REM OpenCAD Launcher - Sets up environment and runs the application

REM Add DLL paths
set PATH=C:\vcpkg\installed\x64-mingw-dynamic\bin;%PATH%
set PATH=C:\Qt\6.10.1\mingw_64\bin;%PATH%
set PATH=C:\Qt\Tools\mingw1310_64\bin;%PATH%

REM Run OpenCAD
"%~dp0build\bin\opencad.exe"
