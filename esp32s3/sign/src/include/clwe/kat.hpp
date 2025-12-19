#ifndef CLWE_KAT_HPP
#define CLWE_KAT_HPP

#include "parameters.hpp"
#include "keygen.hpp"
#include "sign.hpp"
#include <vector>
#include <array>
#include <string>

namespace clwe {

// Structure for Known Answer Test vectors
struct KAT_TestVector {
    uint32_t security_level;
    std::array<uint8_t, 32> seed;           // Seed for deterministic key generation
    std::vector<uint8_t> message;           // Test message
    std::vector<uint8_t> expected_pk;       // Expected public key bytes
    std::vector<uint8_t> expected_sk;       // Expected private key bytes
    std::vector<uint8_t> expected_sig;      // Expected signature bytes
};

// Known Answer Test class
class ColorSignKAT {
private:
    CLWEParameters params_;

    // Embedded test vectors for ML-DSA-44, 65, 87
    static const std::vector<KAT_TestVector> test_vectors_ml_dsa_44;
    static const std::vector<KAT_TestVector> test_vectors_ml_dsa_65;
    static const std::vector<KAT_TestVector> test_vectors_ml_dsa_87;

    // Helper methods
    bool run_keygen_kat(const KAT_TestVector& tv);
    bool run_sign_kat(const KAT_TestVector& tv);
    bool run_verify_kat(const KAT_TestVector& tv);

public:
    ColorSignKAT(const CLWEParameters& params);
    ~ColorSignKAT();

    // Disable copy and assignment
    ColorSignKAT(const ColorSignKAT&) = delete;
    ColorSignKAT& operator=(const ColorSignKAT&) = delete;

    // Run all KATs for the current security level
    bool run_all_kats();

    // Run KATs for specific security level
    static bool run_kats_for_level(uint32_t level);

    // Get test vectors for a security level
    static const std::vector<KAT_TestVector>& get_test_vectors(uint32_t level);
};

// Error codes for KAT operations
enum class ColorSignKATError {
    SUCCESS = 0,
    INVALID_SECURITY_LEVEL,
    KEYGEN_FAILED,
    SIGN_FAILED,
    VERIFY_FAILED,
    INVALID_TEST_VECTOR
};

// Utility function to get error message
std::string get_colorsign_kat_error_message(ColorSignKATError error);

} // namespace clwe

#endif // CLWE_KAT_HPP