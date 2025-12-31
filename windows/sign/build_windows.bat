@echo off
REM Build script for ColorSign Windows using batch commands

setlocal enabledelayedexpansion

echo ========================================
echo ColorSign Windows Build Script
echo ========================================
echo.

REM 1. Find MinGW / GCC
echo Checking for MinGW/GCC...
where gcc >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] GCC found in PATH
    for /f "delims=" %%i in ('where gcc') do set "GCC_PATH=%%i"
    goto found_path
)

echo GCC not in PATH. Checking common locations...

if exist "C:\ProgramData\mingw64\mingw64\bin\gcc.exe" (
    set "MINGW_BIN=C:\ProgramData\mingw64\mingw64\bin"
    set "MINGW_ROOT=C:\ProgramData\mingw64\mingw64"
)

if exist "C:\MinGW\bin\gcc.exe" (
    set "MINGW_BIN=C:\MinGW\bin"
    set "MINGW_ROOT=C:\MinGW"
)

if exist "C:\msys64\mingw64\bin\gcc.exe" (
    set "MINGW_BIN=C:\msys64\mingw64\bin"
    set "MINGW_ROOT=C:\msys64\mingw64"
)

if defined MINGW_BIN (
    echo [OK] Found GCC at !MINGW_BIN!
    set "PATH=!MINGW_BIN!;%PATH%"
) else (
    echo [ERROR] Could not find GCC. Please install MinGW-w64 (e.g., via MSYS2) and add it to PATH.
    exit /b 1
)

:found_path

REM 2. Check for other tools
set "requiredTools=cmake"
for %%t in (%requiredTools%) do (
    where %%t >nul 2>nul
    if !errorlevel! equ 0 (
        echo [OK] %%t found
    ) else (
        echo [ERROR] Required tool '%%t' is not installed or not in PATH.
        exit /b 1
    )
)

echo All required tools are available.
echo.

REM Clean build directory
if exist "build" (
    echo Cleaning build directory...
    rmdir /s /q build
)

REM Create build directory
mkdir build
cd build

REM Run CMake
echo Configuring project with CMake...
REM Pass MINGW_ROOT if we found it, to help the toolchain
if defined MINGW_ROOT (
    set "CMAKE_OPTS=-DMINGW_ROOT=!MINGW_ROOT!"
) else (
    set "CMAKE_OPTS="
)

cmake .. -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE="mingw-toolchain.cmake" !CMAKE_OPTS!
if %errorlevel% neq 0 (
    echo CMake configuration failed.
    echo Try checking your CMakeError.log in build/CMakeFiles/
    exit /b 1
)

REM Build the project
echo Building project...
cmake --build .
if %errorlevel% neq 0 (
    echo Build failed.
    exit /b 1
)

REM Return to original directory
cd ..

echo.
echo Build completed successfully!
echo Executables are available in the build/ directory.

REM Check if we can run the main test
if exist "build\colorsign_test.exe" (
    echo Running quick verification test...
    cd build
    colorsign_test.exe
    if %errorlevel% equ 0 (
        echo Verification test passed!
    ) else (
        echo Warning: Verification test failed.
    )
    cd ..
) else (
    echo Warning: colorsign_test.exe not found.
)

endlocal
