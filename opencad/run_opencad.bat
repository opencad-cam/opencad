@echo off
set QT_BIN=C:\Qt\6.10.1\mingw_64\bin
set PATH=%QT_BIN%;%PATH%
echo Starting OpenCAD...
"build\bin\opencad.exe"
if %errorlevel% neq 0 pause

