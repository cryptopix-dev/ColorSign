# ColorSign: Post-Quantum Digital Signature Library (macOS)

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.16+-blue.svg)](https://cmake.org/)
[![FIPS 204](https://img.shields.io/badge/FIPS-204-green.svg)](https://csrc.nist.gov/pubs/fips/204/final)

ColorSign is a high-performance, production-ready implementation of the Module-Lattice-based Digital Signature Algorithm (ML-DSA) as specified in NIST FIPS 204. This library provides quantum-resistant digital signatures with innovative color-based polynomial encoding for enhanced security and efficiency.

## 🚀 Key Features

- **FIPS 204 Compliant**: Full implementation of ML-DSA (formerly CRYSTALS-Dilithium)
- **Multiple Security Levels**: Support for ML-DSA-44, ML-DSA-65, and ML-DSA-87
- **Color Integration**: Novel polynomial encoding using color representations for improved security
- **High Performance**: SIMD optimizations (AVX2, AVX512, NEON) for accelerated computations
- **Enterprise Ready**: Comprehensive key management, audit logging, and security features
- **COSE Support**: Native CBOR Object Signing and Encryption (COSE) integration
- **macOS Native**: Optimized for macOS with Apple Silicon support
- **Comprehensive Testing**: Full test suite with Known Answer Tests (KAT) vectors

## 📋 Table of Contents

- [Installation](#installation)
- [Quick Start](#quick-start)
- [API Documentation](#api-documentation)
- [Usage Examples](#usage-examples)
- [Testing](#testing)
- [macOS-Specific Features](#macos-specific-features)
- [Build Requirements](#build-requirements)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

## 🔧 Installation

### Prerequisites

- **C++ Compiler**: Clang 10+, GCC 9+, or Apple Clang (Xcode 12+)
- **CMake**: Version 3.16 or higher
- **OpenSSL**: Version 1.1.1 or higher
- **Git**: For cloning dependencies

### macOS Installation

```bash
# Install dependencies using Homebrew
brew update
brew install cmake openssl

# Clone and build
git clone https://github.com/your-org/colorsign.git
cd colorsign

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(sysctl -n hw.ncpu)

# Install (optional)
sudo make install
```

### macOS Package Management

```bash
# Build and install macOS package
cpack -G DragNDrop
```

## 📦 Build Script Documentation

### `build_macos.sh` - macOS Build Script

The `build_macos.sh` script provides a comprehensive, automated build process for ColorSign on macOS systems. It handles macOS-specific requirements, dependency checking, configuration, compilation, and basic verification.

#### Location
```
macos/sign/build_macos.sh
```

#### Features
- **macOS Detection**: Verifies the script is running on macOS (Darwin)
- **Dependency Checking**: Validates required tools (cmake, make, clang, clang++) and OpenSSL development libraries
- **Automated Build**: Configures CMake with Release settings and builds with parallel compilation
- **Verification Testing**: Runs basic tests to ensure the build was successful
- **Error Handling**: Provides clear error messages with macOS-specific installation instructions

#### Usage Instructions

```bash
# Navigate to the macOS sign directory
cd macos/sign

# Make the script executable (if not already)
chmod +x build_macos.sh

# Run the build script
./build_macos.sh
```

#### Command Line Options

The script supports the following usage patterns:

1. **Basic Build** (default):
```bash
./build_macos.sh
```

2. **Debug Build** (modify the script to use `Debug` instead of `Release`):
```bash
# Edit the script to change line 48:
# cmake -DCMAKE_BUILD_TYPE=Debug ..
./build_macos.sh
```

#### What the Script Does

1. **System Validation**: Checks that the script is running on a macOS system
2. **Dependency Verification**: Ensures all required build tools are installed
3. **Build Directory Setup**: Creates a `build/` directory if it doesn't exist
4. **CMake Configuration**: Configures the project with Release build type
5. **Parallel Compilation**: Uses all available CPU cores for faster building
6. **Build Verification**: Runs the main test executable to validate the build
7. **Status Reporting**: Provides clear output about build progress and results

#### Expected Output

```
========================================
ColorSign macOS Build Script
========================================

Checking for required build tools...
All required tools are available.

Configuring project with CMake...
Building project...

Build completed successfully!
Executables are available in the build/ directory:
- colorsign_test: Main ColorSign test executable
- ntt_simd_benchmark: NTT SIMD benchmark tool

Running quick verification test...
[Test output...]
Verification test passed!
```

#### Troubleshooting

- **Missing Dependencies**: The script will provide Homebrew installation commands
- **Xcode Tools**: Ensure Xcode Command Line Tools are installed with `xcode-select --install`
- **OpenSSL Issues**: Use Homebrew to install OpenSSL: `brew install openssl`
- **Permission Issues**: Ensure the script is executable with `chmod +x build_macos.sh`

#### Advanced Usage

For custom build configurations, you can modify the CMake command in the script:

```bash
# Build with Xcode generator
cmake -G Xcode -DCMAKE_BUILD_TYPE=Release ..

# Build for Apple Silicon
cmake -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_BUILD_TYPE=Release ..

# Build universal binary
cmake -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -DCMAKE_BUILD_TYPE=Release ..
```

## 🚀 Quick Start

```cpp
#include "clwe/keygen.hpp"
#include "clwe/sign.hpp"
#include "clwe/verify.hpp"

int main() {
    // Initialize parameters (ML-DSA-44)
    clwe::CLWEParameters params(44);

    // Generate keypair
    clwe::ColorSignKeyGen keygen(params);
    auto [public_key, private_key] = keygen.generate_keypair();

    // Sign a message
    clwe::ColorSign signer(params);
    std::vector<uint8_t> message = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
    auto signature = signer.sign_message(message, private_key, public_key);

    // Verify signature
    clwe::ColorSignVerify verifier(params);
    bool is_valid = verifier.verify_signature(public_key, signature, message);

    return is_valid ? 0 : 1;
}
```

## 📚 API Documentation

### Core Classes

#### `CLWEParameters`
Cryptographic parameter configuration for ML-DSA variants.

```cpp
// Constructor with security level (44, 65, or 87)
clwe::CLWEParameters params(44);

// Custom parameters
clwe::CLWEParameters params(44, 256, 4, 4, 8380417, 2, 39, 78, 524288, 88, 80, 128);
```

#### `ColorSignKeyGen`
Key pair generation functionality.

```cpp
clwe::ColorSignKeyGen keygen(params);
auto [public_key, private_key] = keygen.generate_keypair();

// Deterministic generation (for testing)
auto [pub_key, priv_key] = keygen.generate_keypair_deterministic(seed);
```

#### `ColorSign`
Digital signature creation with enhanced security features.

```cpp
clwe::ColorSign signer(params);

// Basic signing
auto signature = signer.sign_message(message, private_key, public_key);

// COSE signing
auto cose_signature = signer.sign_message_cose(message, private_key, public_key, COSE_ALG_ML_DSA_44);
```

#### `ColorSignVerify`
Signature verification with comprehensive security checks.

```cpp
clwe::ColorSignVerify verifier(params);

// Basic verification
bool is_valid = verifier.verify_signature(public_key, signature, message);

// COSE verification
bool cose_valid = verifier.verify_signature_cose(public_key, cose_signature);
```

### Key Structures

#### `ColorSignPublicKey`
```cpp
struct ColorSignPublicKey {
    std::array<uint8_t, 32> seed_rho;    // Matrix generation seed
    std::array<uint8_t, 32> seed_K;     // Secret key generation seed
    std::array<uint8_t, 64> hash_tr;    // Public key hash
    std::vector<uint8_t> public_data;   // Color-encoded public polynomial
    CLWEParameters params;              // Cryptographic parameters
};
```

#### `ColorSignPrivateKey`
```cpp
struct ColorSignPrivateKey {
    std::array<uint8_t, 32> seed_rho;    // Matrix generation seed
    std::array<uint8_t, 32> seed_K;     // Secret key generation seed
    std::array<uint8_t, 64> hash_tr;    // Public key hash
    std::vector<uint8_t> secret_data;   // Color-encoded secret polynomials
    CLWEParameters params;              // Cryptographic parameters
};
```

## 💡 Usage Examples

### Basic Key Generation and Signing

```cpp
#include "clwe/colorsign.hpp"

int main() {
    // Initialize with ML-DSA-65 for higher security
    clwe::CLWEParameters params(65);

    // Generate keys
    clwe::ColorSignKeyGen keygen(params);
    auto [pub_key, priv_key] = keygen.generate_keypair();

    // Prepare message
    std::string msg = "Quantum-resistant message";
    std::vector<uint8_t> message(msg.begin(), msg.end());

    // Sign message
    clwe::ColorSign signer(params);
    clwe::ColorSignature signature = signer.sign_message(message, priv_key, pub_key);

    // Serialize signature for storage/transmission
    std::vector<uint8_t> sig_bytes = signature.serialize();

    // Verify signature
    clwe::ColorSignVerify verifier(params);
    bool valid = verifier.verify_signature(pub_key, signature, message);

    std::cout << "Signature valid: " << (valid ? "Yes" : "No") << std::endl;
    return 0;
}
```

### COSE Integration

```cpp
#include "clwe/cose.hpp"

// Create COSE Sign1 signature
clwe::COSE_Sign1 cose_sig = signer.sign_message_cose(
    message, private_key, public_key, clwe::COSE_ALG_ML_DSA_44
);

// Serialize to CBOR
std::vector<uint8_t> cose_bytes = cose_sig.serialize();

// Verify COSE signature
bool cose_valid = verifier.verify_signature_cose(public_key, cose_sig);
```

### Enterprise Key Management

```cpp
#include "clwe/enterprise_key_manager.hpp"

// Initialize enterprise key manager
clwe::EnterpriseKeyManager key_manager(config);

// Generate enterprise keypair with lifecycle management
auto key_id = key_manager.generate_enterprise_keypair(
    "production-key-001",
    clwe::KeyType::SIGNING,
    clwe::SecurityLevel::HIGH
);

// Sign with enterprise key
auto signature = key_manager.sign_with_enterprise_key(key_id, message);
```

## 🧪 Testing

### Comprehensive Test Suite

ColorSign for macOS includes a comprehensive test suite with **121 individual tests** across 11 categories, ensuring robust validation of all cryptographic functionality.

### Running All Tests

```bash
# Navigate to macOS sign directory
cd macos/sign

# Run comprehensive test suite (auto-builds if needed)
./run_all_tests.sh
```

The test runner automatically:
- Detects if build is needed and runs `build_macos.sh` if required
- Executes all 121 tests across 11 categories
- Provides color-coded output with detailed statistics
- Returns appropriate exit codes for CI/CD integration

### Test Categories

| Category | Test Count | Description |
|----------|------------|-------------|
| **Parameters** | 6 | Parameter validation and construction |
| **Key Generation** | 15 | Key generation and serialization |
| **Sign** | 13 | Signature creation and validation |
| **Verify** | 16 | Signature verification scenarios |
| **Color Integration** | 13 | Color encoding/decoding |
| **Utils** | 10 | Utility functions and helpers |
| **Security Utils** | 16 | Security-related utilities |
| **Integration** | 11 | End-to-end workflow testing |
| **KAT** | 3 | Known Answer Tests for FIPS compliance |
| **Stress** | 11 | Stress and edge case testing |
| **Main Verification** | 1 | Overall system verification |

**Total: 121 tests providing comprehensive coverage**

### Running Individual Tests

```bash
# After building, run specific test executables
./build/tests/test_parameters
./build/tests/test_keygen
./build/tests/test_sign
./build/tests/test_verify
./build/tests/test_color_integration
./build/tests/test_utils
./build/tests/test_security_utils
./build/tests/test_integration
./build/tests/test_kat
./build/tests/test_stress
```

### Known Answer Tests (KAT)

```bash
# Generate KAT vectors for compliance testing
./build/generate_kat_vectors

# Generate all KAT vectors for all security levels
./build/generate_all_kat_vectors

# Run comprehensive KAT validation
./build/tests/test_kat
```

### Benchmarking

```bash
# Run performance benchmarks
./build/benchmark_color_sign_timing

# SIMD performance test (AVX2/AVX512/NEON)
./build/ntt_simd_benchmark
```

### Test Output Example

```
========================================
ColorSign Comprehensive Test Suite
========================================

Running test suites...
----------------------
Running Parameters Tests... ✓ PASSED
Running Key Generation Tests... ✓ PASSED
Running Sign Tests... ✓ PASSED
Running Verify Tests... ✓ PASSED
Running Color Integration Tests... ✓ PASSED
Running Utils Tests... ✓ PASSED
Running Security Utils Tests... ✓ PASSED
Running Integration Tests... ✓ PASSED
Running KAT Tests... ✓ PASSED
Running Stress Tests... ✓ PASSED
Running Main Verification Test... ✓ PASSED

========================================
Test Summary
========================================
Total Tests: 11
Passed: 11
Failed: 0
All tests passed! ✓
```

### Cross-Platform Testing

The macOS test suite is fully compatible with the Windows and Linux implementations, providing:
- **Identical test coverage** across all platforms
- **Consistent test results** for the same cryptographic operations
- **Platform-specific optimizations** while maintaining functional equivalence
- **Automated CI/CD integration** with clear pass/fail indicators

For more details on cross-platform testing, see [CROSS_PLATFORM_TESTING.md](../CROSS_PLATFORM_TESTING.md).

## 🍎 macOS-Specific Features

### Supported Platforms
- **macOS**: macOS 10.15+ (Catalina and later)
- **Architectures**: x86_64, ARM64 (Apple Silicon M1/M2/M3)

### Platform Features
- **Apple Silicon Support**: Native ARM64 optimization for M1/M2/M3 chips
- **Universal Binary**: Build for both Intel and Apple Silicon
- **Xcode Integration**: Full Xcode project generation support
- **Notarization Ready**: Prepared for macOS app notarization

### Build System
- **CMake**: macOS build configuration with Xcode generator support
- **SIMD Support**: Automatic detection of AVX2, AVX512, and NEON instructions
- **Universal Build**: Support for building universal binaries

### macOS Performance Optimization

```bash
# Build with Xcode generator
cmake -G Xcode ..

# Build for Apple Silicon
cmake -DCMAKE_OSX_ARCHITECTURES=arm64 ..

# Build universal binary
cmake -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" ..

# Enable maximum optimization
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3" ..
```

## 🏗️ Build Requirements

### Minimum Requirements
- **OS**: macOS 10.15+ (Catalina and later)
- **Compiler**: C++17 compliant compiler (Apple Clang 12+)
- **Memory**: 4GB RAM minimum, 8GB recommended
- **Storage**: 500MB for source and build artifacts

### Dependencies
- **OpenSSL**: 1.1.1+ (for cryptographic primitives)
- **CMake**: 3.16+ (build system)
- **GoogleTest**: Automatically downloaded via FetchContent

### SIMD Support
- **x86-64**: AVX2, AVX512F/VL/BW/DQ (automatic detection)
- **ARM64**: NEON (automatic detection)

## 🤝 Contributing

We welcome contributions to ColorSign! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Development Setup

```bash
# Fork and clone
git clone https://github.com/your-org/colorsign.git
cd colorsign

# Create feature branch
git checkout -b feature/your-feature

# Build with tests
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(sysctl -n hw.ncpu)

# Run tests
ctest --output-on-failure
```

### Code Style

- Follow C++17 best practices
- Use RAII principles
- Comprehensive error handling
- Clear documentation comments
- Security-first approach

## 📄 License

ColorSign is licensed under the Apache License 2.0. See [LICENSE](LICENSE) for details.

```
Copyright 2024 ColorSign Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

## 📞 Contact

- **Project Homepage**: https://github.com/cryptopix-dev/colorsign
- **Issues**: https://github.com/cryptopix-dev/colorsign/issues
- **Discussions**: https://github.com/cryptopix-dev/colorsign/discussions
- **Email**: support@cryptopix.in

### Security

For security-related issues, please email support@cryptopix.in instead of creating public issues.

## 🙏 Acknowledgments

- NIST for the FIPS 204 standard
- The CRYSTALS team for the Dilithium algorithm
- OpenSSL project for cryptographic primitives
- GoogleTest framework for testing infrastructure

---

**ColorSign**: Securing the quantum future, one signature at a time.