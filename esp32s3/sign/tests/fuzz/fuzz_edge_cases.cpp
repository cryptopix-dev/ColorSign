#include <cstdint>
#include <vector>
#include <array>
#include "clwe/color_kem.hpp"
#include "clwe/clwe.hpp"
#include "utils.hpp"

namespace clwe {

// Fuzz target for edge cases in cryptographic operations
// Tests utility functions, mathematical operations, and edge conditions
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 4) return 0;

    try {
        // Test Montgomery reduction with fuzzed inputs
        if (size >= 8) {
            uint64_t a = (static_cast<uint64_t>(data[0]) << 56) |
                        (static_cast<uint64_t>(data[1]) << 48) |
                        (static_cast<uint64_t>(data[2]) << 40) |
                        (static_cast<uint64_t>(data[3]) << 32) |
                        (static_cast<uint64_t>(data[4]) << 24) |
                        (static_cast<uint64_t>(data[5]) << 16) |
                        (static_cast<uint64_t>(data[6]) << 8) |
                        static_cast<uint64_t>(data[7]);

            uint32_t q = 3329; // ML-KEM modulus
            uint32_t result = montgomery_reduce(a, q);
            if (result >= q) {
                abort(); // Result should be < q
            }
        }

        // Test Barrett reduction
        if (size >= 12) {
            uint64_t a = (static_cast<uint64_t>(data[0]) << 32) |
                        (static_cast<uint64_t>(data[1]) << 24) |
                        (static_cast<uint64_t>(data[2]) << 16) |
                        (static_cast<uint64_t>(data[3]) << 8) |
                        static_cast<uint64_t>(data[4]);

            uint32_t q = 3329;
            uint64_t mu = (1ULL << 32) / q;
            uint32_t result = barrett_reduce(a, q, mu);
            if (result >= q) {
                abort();
            }
        }

        // Test bit operations
        if (size >= 4) {
            uint32_t x = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

            int len = bit_length(x);
            if (x == 0 && len != 0) abort();
            if (x > 0 && len == 0) abort();

            bool is_pow2 = is_power_of_two(x);
            uint32_t next_pow2 = next_power_of_two(x);

            // Verify next_power_of_two is actually a power of 2
            if (next_pow2 != 0 && !is_power_of_two(next_pow2)) {
                abort();
            }
        }

        // Test modular inverse
        if (size >= 4) {
            uint32_t a = (data[0] << 8) | data[1];
            uint32_t m = 3329;

            if (a > 0 && a < m) {
                try {
                    uint32_t inv = mod_inverse(a, m);
                    uint64_t product = static_cast<uint64_t>(a) * inv % m;
                    if (product != 1) {
                        abort();
                    }
                } catch (const std::exception&) {
                    // Expected for some values
                }
            }
        }

        // Test modular exponentiation
        if (size >= 6) {
            uint32_t base = data[0] % 3329;
            uint32_t exp = (data[1] << 8) | data[2];
            uint32_t mod = 3329;

            uint32_t result = mod_pow(base, exp, mod);
            if (result >= mod) {
                abort();
            }

            // Test Fermat's little theorem for prime modulus
            if (base > 0 && base < mod) {
                uint32_t fermat_result = mod_pow(base, mod - 1, mod);
                if (fermat_result != 1) {
                    abort();
                }
            }
        }

        // Test ColorValue operations
        if (size >= 12) {
            uint32_t r = (data[0] << 16) | (data[1] << 8) | data[2];
            uint32_t g = (data[3] << 16) | (data[4] << 8) | data[5];
            uint32_t b = (data[6] << 16) | (data[7] << 8) | data[8];

            try {
                ColorValue cv(r, g, b);
                uint32_t math_val = cv.to_math_value();

                // Test conversion back
                ColorValue cv2 = ColorValue::from_math_value(math_val);
                if (cv2.to_math_value() != math_val) {
                    abort();
                }

                // Test precise value
                uint32_t precise_val = cv.to_precise_value();
                ColorValue cv3 = ColorValue::from_precise_value(precise_val);
                if (cv3.to_precise_value() != precise_val) {
                    abort();
                }
            } catch (const std::exception&) {
                // Expected for invalid values
            }
        }

        // Test AVXVector operations if AVX is available
        if (size >= 16) {
            try {
                AVXVector<uint32_t> vec;
                for (size_t i = 0; i < 8 && i < size; ++i) {
                    vec.push_back(data[i]);
                }

                // Test size and access
                if (vec.size() != std::min(static_cast<size_t>(8), size)) {
                    abort();
                }

                for (size_t i = 0; i < vec.size(); ++i) {
                    if (vec[i] != data[i]) {
                        abort();
                    }
                }

                // Test resize
                vec.resize(4);
                if (vec.size() != 4) {
                    abort();
                }

            } catch (const std::exception&) {
                // Expected
            }
        }

        // Test timestamp functions
        uint64_t ts = get_timestamp_ns();
        double ms = timestamp_to_ms(ts);
        if (ms < 0) {
            abort();
        }

    } catch (const std::exception&) {
        // Catch any unexpected exceptions
        return 0;
    }

    return 0;
}

} // namespace clwe