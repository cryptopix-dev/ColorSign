@echo off
setlocal enabledelayedexpansion

:: ColorSign Benchmark Runner
:: Runs all available benchmark executables and reports results

echo ======================================
echo ColorSign Benchmark Suite
echo ======================================
echo.

:: Check if build directory exists
if not exist "build\" (
    echo Error: Build directory not found. Please run build_windows.bat first.
    exit /b 1
)

:: Check if benchmark executables exist
if not exist "build\benchmark_color_sign_timing.exe" (
    echo Error: benchmark_color_sign_timing.exe not found in build directory.
    exit /b 1
)

if not exist "build\ntt_simd_benchmark.exe" (
    echo Error: ntt_simd_benchmark.exe not found in build directory.
    exit /b 1
)

echo Running ColorSign Timing Benchmark...
echo --------------------------------------
echo.
call build\benchmark_color_sign_timing.exe
if %errorlevel% neq 0 (
    echo Error: benchmark_color_sign_timing.exe failed with exit code %errorlevel%
    exit /b %errorlevel%
)

echo.
echo Running NTT SIMD Benchmark...
echo --------------------------------------
echo.
call build\ntt_simd_benchmark.exe
if %errorlevel% neq 0 (
    echo Error: ntt_simd_benchmark.exe failed with exit code %errorlevel%
    exit /b %errorlevel%
)

echo.
echo ======================================
echo Benchmark Summary
echo ======================================
echo All benchmarks completed successfully!
echo.
echo Benchmark executables run:
echo   - benchmark_color_sign_timing.exe
echo   - ntt_simd_benchmark.exe
echo.
echo For detailed results, see output above.