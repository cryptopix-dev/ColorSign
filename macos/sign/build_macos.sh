#!/bin/bash

# Build script for ColorSign macOS
# This script builds the ColorSign algorithm on macOS systems

set -e

echo "========================================"
echo "ColorSign macOS Build Script"
echo "========================================"
echo

# Check if we're on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "Error: This build script is designed for macOS systems only."
    echo "Detected OS: $OSTYPE"
    exit 1
fi

# Check for required tools
echo "Checking for required build tools..."
for tool in cmake make clang clang++; do
    if ! command -v $tool &> /dev/null; then
        echo "Error: Required tool '$tool' is not installed."
        echo "Please install Xcode Command Line Tools:"
        echo "  xcode-select --install"
        exit 1
    fi
done

# Check for OpenSSL development libraries
if ! pkg-config --exists openssl; then
    echo "Error: OpenSSL development libraries are not installed."
    echo "Please install them using Homebrew:"
    echo "  brew install openssl"
    echo "  export PKG_CONFIG_PATH=/usr/local/opt/openssl/lib/pkgconfig:$PKG_CONFIG_PATH"
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
make -j$(sysctl -n hw.ncpu)

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