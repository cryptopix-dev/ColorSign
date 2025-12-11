cmake_minimum_required(VERSION 3.16)
project(ColorSign VERSION 1.0.0 LANGUAGES C CXX)

# Compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -ffast-math -funroll-loops -fomit-frame-pointer")

# SIMD optimization flags (only for x86-64)
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mavx512f -mavx512vl -mavx512bw -mavx512dq")
    add_definitions(-DHAVE_AVX2 -DHAVE_AVX512)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    # For ARM64, enable NEON instead of AVX
    add_definitions(-DHAVE_NEON)
endif()

# Dependencies
find_package(OpenSSL REQUIRED)

# OpenSSL include directories
include_directories(${OPENSSL_INCLUDE_DIR})

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# Configure GoogleTest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Suppress CMake deprecation warnings from googletest
set(CMAKE_POLICY_VERSION_MINIMUM 3.16)
FetchContent_MakeAvailable(googletest)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/core)

# Library target
add_library(colorsign STATIC
    src/core/parameters.cpp
    src/core/keygen.cpp
    src/core/sign.cpp
    src/core/verify.cpp
    src/core/cose.cpp
    src/core/color_integration.cpp
    src/core/utils.cpp
    src/core/security_utils.cpp
    src/core/kat.cpp
    src/core/cpu_features.cpp
    src/core/ntt_engine.cpp
    src/core/version.cpp
)

target_link_libraries(colorsign PRIVATE OpenSSL::Crypto)

# Main executable
add_executable(colorsign_test src/main.cpp)
target_link_libraries(colorsign_test PRIVATE colorsign)

# Benchmark executable
add_executable(benchmark_color_sign_timing benchmark_color_sign_timing.cpp)
target_link_libraries(benchmark_color_sign_timing PRIVATE colorsign)

# SIMD benchmark executable
add_executable(ntt_simd_benchmark src/core/ntt_simd_benchmark.cpp)
target_link_libraries(ntt_simd_benchmark PRIVATE colorsign)

# KAT vector generator executable
add_executable(generate_kat_vectors generate_kat_vectors.cpp)
target_link_libraries(generate_kat_vectors PRIVATE colorsign)

# All KAT vectors generator executable
add_executable(generate_all_kat_vectors generate_all_kat_vectors.cpp)
target_link_libraries(generate_all_kat_vectors PRIVATE colorsign)

# Tests
enable_testing()

# Add tests subdirectory
add_subdirectory(tests)

# Add comprehensive test target - run all tests including unit tests
add_custom_target(run_all_tests
    COMMAND ${CMAKE_COMMAND} -E echo "Running basic verification test..."
    COMMAND ./colorsign_test
    COMMAND ${CMAKE_COMMAND} -E echo "Running unit tests..."
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running ColorSign verification and unit tests"
)

# Add benchmark as a test
add_test(NAME BenchmarkTest COMMAND benchmark_color_sign_timing)

# Installation
install(TARGETS colorsign
    EXPORT ColorSignTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

install(DIRECTORY src/include/clwe/
    DESTINATION include/clwe
    FILES_MATCHING PATTERN "*.hpp"
)

# Export targets
install(EXPORT ColorSignTargets
    FILE ColorSignTargets.cmake
    NAMESPACE ColorSign::
    DESTINATION lib/cmake/ColorSign
)