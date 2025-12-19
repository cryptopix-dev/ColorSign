#include "../include/clwe/parameters.hpp"

namespace clwe {

// Validation function implementation
void CLWEParameters::validate() const {
    // Validate security level
    if (security_level != 44 && security_level != 65 && security_level != 87) {
        throw std::invalid_argument("Invalid security level: must be 44, 65, or 87");
    }

    // Validate degree: must be power of 2 and reasonable
    if (degree == 0 || (degree & (degree - 1)) != 0 || degree > 8192) {
        throw std::invalid_argument("Invalid degree: must be a power of 2 between 1 and 8192");
    }

    // Validate module rank: positive and reasonable
    if (module_rank == 0 || module_rank > 16) {
        throw std::invalid_argument("Invalid module rank: must be between 1 and 16");
    }

    // Validate repetitions: positive and reasonable
    if (repetitions == 0 || repetitions > 16) {
        throw std::invalid_argument("Invalid repetitions: must be between 1 and 16");
    }

    // Validate modulus: must be prime and appropriate size
    if (!is_prime(modulus) || modulus < 256 || modulus > 16777216) {  // Allow up to 2^24
        throw std::invalid_argument("Invalid modulus: must be a prime between 256 and 16777216");
    }

    // Validate eta: positive and reasonable
    if (eta == 0 || eta > 16) {
        throw std::invalid_argument("Invalid eta: must be between 1 and 16");
    }

    // Validate tau: positive and reasonable
    if (tau == 0 || tau > degree) {
        throw std::invalid_argument("Invalid tau: must be between 1 and degree");
    }

    // Validate beta: positive
    if (beta == 0) {
        throw std::invalid_argument("Invalid beta: must be positive");
    }

    // Validate gamma1: positive and reasonable
    if (gamma1 == 0 || gamma1 > (1 << 20)) {
        throw std::invalid_argument("Invalid gamma1: must be between 1 and 2^20");
    }

    // Validate gamma2: positive
    if (gamma2 == 0) {
        throw std::invalid_argument("Invalid gamma2: must be positive");
    }

    // Validate omega: positive
    if (omega == 0) {
        throw std::invalid_argument("Invalid omega: must be positive");
    }

    // Validate lambda: valid security strength
    if (lambda != 128 && lambda != 192 && lambda != 256) {
        throw std::invalid_argument("Invalid lambda: must be 128, 192, or 256");
    }
}

} // namespace clwe