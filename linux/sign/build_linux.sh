#!/bin/bash

# Build script for ColorSign Linux
# This script builds the ColorSign algorithm on Linux systems

set -e

echo "========================================"
echo "ColorSign Linux Build Script"
echo "========================================"
echo

# Check if we're on Linux
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo "Error: This build script is designed for Linux systems only."
    echo "Detected OS: $OSTYPE"
    exit 1
fi

# Check for required tools
echo "Checking for required build tools..."
for tool in cmake make gcc g++; do
    if ! command -v $tool &> /dev/null; then
        echo "Error: Required tool '$tool' is not installed."
        echo "Please install it using your package manager:"
        echo "  sudo apt-get install $tool  # Debian/Ubuntu"
        echo "  sudo yum install $tool      # RHEL/CentOS"
        exit 1
    fi
done

# Check for OpenSSL development libraries
if ! pkg-config --exists openssl; then
    echo "Error: OpenSSL development libraries are not installed."
    echo "Please install them using your package manager:"
    echo "  sudo apt-get install libssl-dev  # Debian/Ubuntu"
    echo "  sudo yum install openssl-devel   # RHEL/CentOS"
    exit 1
fi

echo "All required tools are available."
echo

# Create build directory
mkdir -p build
cd build

echo "Configuring project with CMake..."
cmake -DCMAKE_BUILD_TYPE=Release ..

echo "Building project..."
make -j$(nproc)

echo
echo "Build completed successfully!"
echo "Executables are available in the build/ directory:"
echo "  - colorsign_test: Main ColorSign test executable"
echo "  - ntt_simd_benchmark: NTT SIMD benchmark tool"
echo

# Check if we can run the main test
if [ -f colorsign_test ]; then
    echo "Running quick verification test..."
    ./colorsign_test
    if [ $? -eq 0 ]; then
        echo "Verification test passed!"
    else
        echo "Warning: Verification test failed."
    fi
else
    echo "Warning: colorsign_test executable not found."
fi

cd ..