#!/bin/bash

# ColorSign Comprehensive Test Runner
# This script runs all test suites and provides a summary

# set -e

echo "========================================"
echo "ColorSign Comprehensive Test Suite"
echo "========================================"
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test results
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

run_test() {
    local test_name=$1
    local test_command=$2

    echo -n "Running $test_name... "

    if eval "$test_command"; then
        echo -e "${GREEN}PASSED${NC}"
        ((PASSED_TESTS++))
    else
        echo -e "${RED}FAILED${NC}"
        ((FAILED_TESTS++))
        # Run again to show output
        echo "Test output:"
        eval "$test_command"
        echo
    fi
    ((TOTAL_TESTS++))
}

echo "Building tests..."
if [ ! -d "build" ]; then
    echo "Build directory not found. Running build script first..."
    ./build_macos.sh
fi

cd build
if [[ "$OSTYPE" == "darwin"* ]]; then
    make -j$(sysctl -n hw.ncpu)
else
    make -j$(nproc)
fi
cd ..

echo
echo "Running test suites..."
echo "----------------------"

# Run individual test suites
run_test "Parameters Tests" "./build/tests/test_parameters"
run_test "Key Generation Tests" "./build/tests/test_keygen"
run_test "Sign Tests" "./build/tests/test_sign"
run_test "Verify Tests" "./build/tests/test_verify"
run_test "Color Integration Tests" "./build/tests/test_color_integration"
run_test "Utils Tests" "./build/tests/test_utils"
run_test "Security Utils Tests" "./build/tests/test_security_utils"
run_test "Integration Tests" "./build/tests/test_integration"
run_test "KAT Tests" "./build/tests/test_kat"
run_test "Stress Tests" "./build/tests/test_stress"

# Run main verification test
run_test "Main Verification Test" "./build/colorsign_test"

echo
echo "========================================"
echo "Test Summary"
echo "========================================"
echo "Total Tests: $TOTAL_TESTS"
echo -e "Passed: ${GREEN}$PASSED_TESTS${NC}"
echo -e "Failed: ${RED}$FAILED_TESTS${NC}"

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}All tests passed! ✓${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed! ✗${NC}"
    exit 1
fi