@echo off
cls
title Quick Incremental Build

echo ================================
echo    Quick Incremental Build
echo ================================
echo.

if not exist "CMakeLists.txt" (
    echo Error: CMakeLists.txt not found!
    pause
    exit /b 1
)

if not exist "build" mkdir build
cd build

REM Only reconfigure if essential files don't exist
if not exist "Makefile" (
    echo [1/2] Configuring project...
    cmake .. -G "MinGW Makefiles"
    if %ERRORLEVEL% NEQ 0 (
        echo Error: CMake configuration failed!
        cd ..
        pause
        exit /b 1
    )
) else (
    echo [1/2] Using cached configuration...
)

echo [2/2] Compiling with 4 parallel jobs...
mingw32-make -j4
if %ERRORLEVEL% NEQ 0 (
    echo Error: Compilation failed!
    cd ..
    pause
    exit /b 1
)

cd ..
echo Build completed successfully!
pause