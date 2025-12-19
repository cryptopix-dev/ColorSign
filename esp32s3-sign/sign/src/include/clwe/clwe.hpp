#ifndef CLWE_HPP
#define CLWE_HPP

#include <cstdint>
#include <stdexcept>
#include <string>

// Main CLWE namespace
namespace clwe {

// Version information
const std::string VERSION = "1.0.0";

// Parameter structure for CLWE operations
struct CLWEParameters {
    uint32_t security_level;  // Security level (512, 768, 1024)
    uint32_t degree;          // Ring degree (power of 2)
    uint32_t module_rank;     // Module rank k
    uint32_t modulus;         // Prime modulus q
    uint32_t eta1;           // Binomial distribution parameter for key generation
    uint32_t eta2;           // Binomial distribution parameter for encryption

    // Constructor with defaults - NIST-standard ML-KEM parameters
    CLWEParameters(uint32_t sec_level = 512)
        : security_level(sec_level), degree(256), module_rank(2),
          modulus(3329), eta1(3), eta2(2) {  // ML-KEM modulus: 2^12 + 1
        // Set parameters based on security level (ML-KEM)
        switch (sec_level) {
            case 512:  // ML-KEM-512
                degree = 256;
                module_rank = 2;
                modulus = 3329;  // q = 2^12 + 1
                eta1 = 3;
                eta2 = 2;
                break;
            case 768:  // ML-KEM-768
                degree = 256;
                module_rank = 3;
                modulus = 3329;
                eta1 = 2;
                eta2 = 2;
                break;
            case 1024:  // ML-KEM-1024
                degree = 256;
                module_rank = 4;
                modulus = 3329;
                eta1 = 2;
                eta2 = 2;
                break;
            default:
                // Keep defaults
                break;
        }
        validate();
    }

    // Constructor with custom parameters
    CLWEParameters(uint32_t sec_level, uint32_t deg, uint32_t rank, uint32_t mod, uint32_t e1, uint32_t e2)
        : security_level(sec_level), degree(deg), module_rank(rank),
          modulus(mod), eta1(e1), eta2(e2) {
        validate();
    }

    // Validation function
    void validate() const {
        // Validate security level
        if (security_level != 512 && security_level != 768 && security_level != 1024) {
            throw std::invalid_argument("Invalid security level: must be 512, 768, or 1024");
        }

        // Validate degree: must be power of 2 and reasonable
        if (degree == 0 || (degree & (degree - 1)) != 0 || degree > 8192) {
            throw std::invalid_argument("Invalid degree: must be a power of 2 between 1 and 8192");
        }

        // Validate module rank: positive and reasonable
        if (module_rank == 0 || module_rank > 16) {
            throw std::invalid_argument("Invalid module rank: must be between 1 and 16");
        }

        // Validate modulus: must be prime and appropriate size
        if (!is_prime(modulus) || modulus < 256 || modulus > 65536) {
            throw std::invalid_argument("Invalid modulus: must be a prime between 256 and 65536");
        }

        // Validate eta1: positive and reasonable
        if (eta1 == 0 || eta1 > 16) {
            throw std::invalid_argument("Invalid eta1: must be between 1 and 16");
        }

        // Validate eta2: positive and reasonable
        if (eta2 == 0 || eta2 > 16) {
            throw std::invalid_argument("Invalid eta2: must be between 1 and 16");
        }
    }

    // Helper function to check if a number is prime
    static bool is_prime(uint32_t n) {
        if (n <= 1) return false;
        if (n <= 3) return true;
        if (n % 2 == 0 || n % 3 == 0) return false;
        for (uint32_t i = 5; i * i <= n; i += 6) {
            if (n % i == 0 || n % (i + 2) == 0) return false;
        }
        return true;
    }

private:
};

// Error codes
enum class CLWEError {
    SUCCESS = 0,
    INVALID_PARAMETERS = 1,
    MEMORY_ALLOCATION_FAILED = 2,
    AVX_NOT_SUPPORTED = 3,
    INVALID_KEY = 4,
    VERIFICATION_FAILED = 5,
    UNKNOWN_ERROR = 6
};

// Utility function to get error message
std::string get_error_message(CLWEError error);

} // namespace clwe

#endif // CLWE_HPP