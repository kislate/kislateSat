@echo off
echo Cleaning build files...

if exist "build" (
    echo Removing build directory...
    rmdir /s /q build
)

if exist "bin" (
    echo Removing bin directory...
    rmdir /s /q bin
)

echo Clean completed!
pause