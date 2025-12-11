@echo off
echo ========================================
ColorSign Comprehensive Test Suite
========================================
echo.

set TotalTests=1
set PassedTests=0
set FailedTests=0

if not exist "build" (
    echo Build directory not found. Please run build_windows.bat first.
    exit /b 1
)

echo.
echo Running test suites...
echo ----------------------

echo Running ColorSign comprehensive test...
call build\colorsign_test.exe
if %errorlevel% equ 0 (
    echo PASSED
    set /a PassedTests+=1
) else (
    echo FAILED
    set /a FailedTests+=1
)

echo.
echo ========================================
echo Test Summary
========================================
echo Total Tests: %TotalTests%
echo Passed: %PassedTests%
echo Failed: %FailedTests%

if %FailedTests% equ 0 (
    echo All tests passed!
    exit /b 0
) else (
    echo Some tests failed!
    exit /b 1
)