#include <cstdint>
#include <vector>
#include <array>
#include "clwe/color_kem.hpp"
#include "clwe/clwe.hpp"

namespace clwe {

// Fuzz target for input validation functions
// Tests parameter validation, key validation, and other input validation
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 4) return 0; // Need at least 4 bytes for basic testing

    try {
        // Test parameter validation with fuzzed data
        uint32_t security_level = (data[0] << 16) | (data[1] << 8) | data[2];
        uint32_t degree = (data[3] << 8) | data[4];
        uint32_t modulus = (data[5] << 8) | data[6];
        uint32_t eta = data[7];
        uint32_t eta_b = data[8];
        uint32_t tau = data[9];

        // Try to create parameters with fuzzed values
        try {
            CLWEParameters params(security_level, degree, eta, modulus, eta_b, tau);
            // If parameters are valid, try to create a KEM instance
            ColorKEM kem(params);
        } catch (const std::exception&) {
            // Expected for invalid parameters
        }

        // Test key validation with fuzzed data
        if (size >= 32 + 4) {
            std::array<uint8_t, 32> seed;
            std::copy(data + 10, data + 42, seed.begin());

            std::vector<uint8_t> public_data(data + 42, data + size);

            try {
                CLWEParameters valid_params(512); // Use valid parameters
                ColorPublicKey pub_key(seed, public_data, valid_params);

                // Try to create KEM and validate key
                ColorKEM kem(valid_params);
                // Note: verify_keypair requires both keys, so we can't fully test here
                // But the key construction itself tests some validation
            } catch (const std::exception&) {
                // Expected for invalid keys
            }
        }

        // Test color value validation
        if (size >= 12) {
            uint32_t r = (data[0] << 16) | (data[1] << 8) | data[2];
            uint32_t g = (data[3] << 16) | (data[4] << 8) | data[5];
            uint32_t b = (data[6] << 16) | (data[7] << 8) | data[8];

            try {
                ColorValue cv(r, g, b);
                // Test that values are within valid range
                if (cv.to_math_value() >= 3329) { // ML-KEM modulus
                    // This should not happen with valid ColorValue
                    abort();
                }
            } catch (const std::exception&) {
                // Expected for invalid color values
            }
        }

    } catch (const std::exception&) {
        // Catch any unexpected exceptions
        return 0;
    }

    return 0;
}

} // namespace clwe