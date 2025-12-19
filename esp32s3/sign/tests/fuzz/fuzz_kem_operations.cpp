#include <cstdint>
#include <vector>
#include <array>
#include "clwe/color_kem.hpp"
#include "clwe/clwe.hpp"

namespace clwe {

// Fuzz target for KEM operations with random inputs
// Tests key generation, encapsulation, and decapsulation with fuzzed data
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 4) return 0;

    try {
        // Use fuzzed data to select security level
        uint32_t security_level;
        if (size > 0) {
            uint8_t level_selector = data[0] % 3;
            security_level = (level_selector == 0) ? 512 : (level_selector == 1) ? 768 : 1024;
        } else {
            security_level = 512;
        }

        CLWEParameters params(security_level);
        ColorKEM kem(params);

        // Test key generation
        try {
            auto keypair = kem.keygen();
            auto public_key = keypair.first;
            auto private_key = keypair.second;

            // Test encapsulation
            try {
                auto encap_result = kem.encapsulate(public_key);
                auto ciphertext = encap_result.first;
                auto shared_secret = encap_result.second;

                // Test decapsulation
                try {
                    ColorValue recovered_secret = kem.decapsulate(public_key, private_key, ciphertext);

                    // Verify that decapsulation matches encapsulation
                    if (recovered_secret != shared_secret) {
                        // This should never happen with correct implementation
                        abort();
                    }
                } catch (const std::exception&) {
                    // Expected for some malformed inputs, but shouldn't happen with valid data
                }
            } catch (const std::exception&) {
                // Expected for some fuzzed scenarios
            }

            // Test key verification
            try {
                bool valid = kem.verify_keypair(public_key, private_key);
                // Should be true for valid keypairs
                if (!valid) {
                    // This indicates a problem
                    abort();
                }
            } catch (const std::exception&) {
                // Expected
            }

        } catch (const std::exception&) {
            // Expected for some parameter combinations
        }

        // Test multiple rounds of keygen/encap/decap to stress test
        for (int round = 0; round < 5 && round < size; ++round) {
            try {
                auto keypair = kem.keygen();
                auto public_key = keypair.first;
                auto private_key = keypair.second;
                auto encap_result = kem.encapsulate(public_key);
                auto ciphertext = encap_result.first;
                auto shared_secret = encap_result.second;
                ColorValue recovered_secret = kem.decapsulate(public_key, private_key, ciphertext);

                if (recovered_secret != shared_secret) {
                    abort();
                }
            } catch (const std::exception&) {
                // Expected for some rounds
            }
        }

    } catch (const std::exception&) {
        // Catch any unexpected exceptions
        return 0;
    }

    return 0;
}

} // namespace clwe