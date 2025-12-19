/**
 * @file color_kem.hpp
 * @brief ColorKEM: Post-Quantum Key Encapsulation with Color Visualization
 *
 * This header defines the ColorKEM key encapsulation mechanism, which provides
 * post-quantum secure key exchange using lattice-based cryptography with a
 * unique color-based coefficient representation.
 *
 * ColorKEM is mathematically equivalent to ML-KEM (NIST FIPS 203) but represents
 * polynomial coefficients as RGBA color values for visualization and debugging
 * purposes. The cryptographic security remains identical to standard lattice-based
 * schemes.
 *
 * @author ColorKEM Development Team
 * @version 1.0.0
 * @date 2024
 *
 * @see https://doi.org/10.6028/NIST.FIPS.203 (ML-KEM specification)
 */

#ifndef COLOR_KEM_HPP
#define COLOR_KEM_HPP

#include "clwe/clwe.hpp"
#include "clwe/color_value.hpp"
#include "clwe/color_ntt_engine.hpp"
#include <vector>
#include <array>
#include <memory>

namespace clwe {

// Forward declarations
class ColorNTTEngine;
class ColorKEM;

/**
 * @brief Public key structure for ColorKEM
 *
 * Contains the public key components needed for encapsulation:
 * - A 32-byte seed used to deterministically generate the matrix A
 * - Serialized public key data (polynomial coefficients as colors)
 * - Cryptographic parameters
 *
 * @note The public key can be safely shared and does not contain sensitive information.
 */
struct ColorPublicKey {
    std::array<uint8_t, 32> seed;
    std::vector<uint8_t> public_data;
    CLWEParameters params;

    ColorPublicKey() = default;
    ColorPublicKey(const std::array<uint8_t, 32>& s, const std::vector<uint8_t>& pd, const CLWEParameters& p)
        : seed(s), public_data(pd), params(p) {}

    std::vector<uint8_t> serialize() const;
    static ColorPublicKey deserialize(const std::vector<uint8_t>& data, const CLWEParameters& params);
};

/**
 * @brief Private key structure for ColorKEM
 *
 * Contains the sensitive private key data needed for decapsulation:
 * - Serialized secret key polynomials (coefficients as colors)
 * - Cryptographic parameters
 *
 * @warning The private key must be kept secret and protected from unauthorized access.
 * @note Private keys should be securely erased from memory after use.
 */
struct ColorPrivateKey {
    std::vector<uint8_t> secret_data;
    CLWEParameters params;

    ColorPrivateKey() = default;
    ColorPrivateKey(const std::vector<uint8_t>& sd, const CLWEParameters& p)
        : secret_data(sd), params(p) {}

    std::vector<uint8_t> serialize() const;
    static ColorPrivateKey deserialize(const std::vector<uint8_t>& data, const CLWEParameters& params);
};

/**
 * @brief Ciphertext structure for ColorKEM
 *
 * Contains the encapsulated ciphertext and shared secret hint:
 * - Encrypted message data (polynomial coefficients as colors)
 * - Hint for the shared secret (used for verification)
 * - Cryptographic parameters
 *
 * @note Ciphertexts can be safely transmitted over public channels.
 */
struct ColorCiphertext {
    std::vector<uint8_t> ciphertext_data;
    std::vector<uint8_t> shared_secret_hint;
    CLWEParameters params;

    ColorCiphertext() = default;
    ColorCiphertext(const std::vector<uint8_t>& cd, const std::vector<uint8_t>& ssh, const CLWEParameters& p)
        : ciphertext_data(cd), shared_secret_hint(ssh), params(p) {}

    std::vector<uint8_t> serialize() const;
    static ColorCiphertext deserialize(const std::vector<uint8_t>& data);
    static ColorCiphertext deserialize(const std::vector<uint8_t>& data, const CLWEParameters& params);
};

/**
 * @brief Main ColorKEM key encapsulation mechanism implementation
 *
 * ColorKEM provides IND-CCA2 secure post-quantum key encapsulation using
 * lattice-based cryptography. The implementation uses color values (RGBA)
 * to represent polynomial coefficients, enabling visual debugging while
 * maintaining mathematical equivalence to standard ML-KEM.
 *
 * Key Features:
 * - Post-quantum security based on the Learning With Errors problem
 * - Compatible with ML-KEM security levels (512, 768, 1024)
 * - SIMD acceleration (AVX-512, AVX2, NEON)
 * - Constant-time operations for side-channel resistance
 * - Comprehensive input validation and error handling
 *
 * Example usage:
 * @code
 * clwe::CLWEParameters params(768);  // ML-KEM-768
 * clwe::ColorKEM kem(params);
 *
 * auto [pk, sk] = kem.keygen();
 * auto [ct, ss] = kem.encapsulate(pk);
 * clwe::ColorValue recovered_ss = kem.decapsulate(pk, sk, ct);
 * @endcode
 *
 * @note All operations are thread-safe for read-only access.
 * @warning This class is not copyable due to internal state management.
 */
class ColorKEM {
private:
    CLWEParameters params_;
    std::unique_ptr<ColorNTTEngine> color_ntt_engine_;

    // Helper methods
    std::vector<std::vector<std::vector<ColorValue>>> generate_matrix_A(const std::array<uint8_t, 32>& seed) const;
    std::vector<std::vector<ColorValue>> generate_error_vector(uint32_t eta) const;
    std::vector<std::vector<ColorValue>> generate_secret_key(uint32_t eta) const;
    // Deterministic versions for KATs
    std::vector<std::vector<ColorValue>> generate_error_vector_deterministic(uint32_t eta, const std::array<uint8_t, 32>& seed) const;
    std::vector<std::vector<ColorValue>> generate_secret_key_deterministic(uint32_t eta, const std::array<uint8_t, 32>& seed) const;
    std::vector<std::vector<ColorValue>> generate_public_key(const std::vector<std::vector<ColorValue>>& secret_key,
                                                const std::vector<std::vector<std::vector<ColorValue>>>& matrix_A,
                                                const std::vector<std::vector<ColorValue>>& error_vector) const;
    std::vector<std::vector<ColorValue>> matrix_vector_mul(const std::vector<std::vector<std::vector<ColorValue>>>& matrix,
                                              const std::vector<std::vector<ColorValue>>& vector) const;
    std::vector<std::vector<ColorValue>> matrix_transpose_vector_mul(const std::vector<std::vector<std::vector<ColorValue>>>& matrix,
                                                        const std::vector<std::vector<ColorValue>>& vector) const;
    ColorValue decrypt_message(const std::vector<std::vector<ColorValue>>& secret_key,
                              const std::vector<std::vector<ColorValue>>& ciphertext) const;
    ColorValue generate_shared_secret() const;
    std::vector<uint8_t> encode_color_secret(const ColorValue& secret) const;
    ColorValue decode_color_secret(const std::vector<uint8_t>& encoded) const;
    std::vector<uint8_t> color_secret_to_bytes(const ColorValue& secret);
    ColorValue bytes_to_color_secret(const std::vector<uint8_t>& bytes);
    std::vector<std::vector<ColorValue>> encrypt_message(const std::vector<std::vector<std::vector<ColorValue>>>& matrix_A,
                                           const std::vector<std::vector<ColorValue>>& public_key,
                                           const ColorValue& message) const;
    std::vector<std::vector<ColorValue>> encrypt_message_deterministic(const std::vector<std::vector<std::vector<ColorValue>>>& matrix_A,
                                                          const std::vector<std::vector<ColorValue>>& public_key,
                                                          const ColorValue& message,
                                                          const std::array<uint8_t, 32>& r_seed,
                                                          const std::array<uint8_t, 32>& e1_seed,
                                                          const std::array<uint8_t, 32>& e2_seed) const;
    ColorValue hash_ciphertext(const ColorCiphertext& ciphertext) const;

public:
    /**
     * @brief Construct a new ColorKEM instance
     *
     * Initializes the KEM with the specified cryptographic parameters.
     * The constructor validates parameters and initializes the NTT engine.
     *
     * @param params Cryptographic parameters (security level, modulus, etc.)
     *
     * @throws std::invalid_argument If parameters are invalid
     * @throws std::runtime_error If NTT engine initialization fails
     *
     * @note Parameter validation is performed during construction.
     * @see CLWEParameters for parameter details
     */
    ColorKEM(const CLWEParameters& params);

    /**
     * @brief Destroy the ColorKEM instance
     *
     * Properly cleans up internal resources and securely erases
     * any sensitive data from memory.
     */
    ~ColorKEM();

    // Disable copy and assignment for security reasons
    ColorKEM(const ColorKEM&) = delete;             /**< Copy constructor disabled */
    ColorKEM& operator=(const ColorKEM&) = delete;  /**< Copy assignment disabled */

    /**
     * @brief Generate a new key pair
     *
     * Creates a public-private key pair using the configured parameters.
     * The key generation process includes:
     * - Generation of random matrix seed
     * - Sampling of secret key polynomials
     * - Computation of public key polynomials
     * - Application of error correction
     *
     * @return std::pair<ColorPublicKey, ColorPrivateKey> Public and private key pair
     *
     * @throws std::runtime_error If key generation fails due to insufficient entropy
     *
     * @note This operation requires cryptographically secure random number generation.
     * @warning Key generation is computationally intensive and may take several milliseconds.
     */
    std::pair<ColorPublicKey, ColorPrivateKey> keygen();

    /**
     * @brief Encapsulate a shared secret
     *
     * Generates a random shared secret and encapsulates it into a ciphertext
     * that can only be decapsulated by the holder of the corresponding private key.
     *
     * The encapsulation process:
     * 1. Generates a random shared secret
     * 2. Samples random polynomials for encryption
     * 3. Computes ciphertext using public key
     * 4. Returns (ciphertext, shared_secret) pair
     *
     * @param public_key The recipient's public key
     * @return std::pair<ColorCiphertext, ColorValue> Ciphertext and encapsulated shared secret
     *
     * @throws std::invalid_argument If public key parameters don't match KEM instance
     * @throws std::invalid_argument If public key data is malformed
     *
     * @note The shared secret is a single ColorValue representing the encapsulated key.
     * @see decapsulate() for the corresponding decapsulation operation
     */
    std::pair<ColorCiphertext, ColorValue> encapsulate(const ColorPublicKey& public_key);

    /**
     * @brief Decapsulate a shared secret
     *
     * Recovers the shared secret from a ciphertext using the corresponding private key.
     * This operation can only be performed by the holder of the private key.
     *
     * The decapsulation process:
     * 1. Validates input parameters
     * 2. Decrypts the ciphertext using the private key
     * 3. Performs error correction and verification
     * 4. Returns the recovered shared secret
     *
     * @param public_key The corresponding public key (for verification)
     * @param private_key The private key for decapsulation
     * @param ciphertext The ciphertext to decapsulate
     * @return ColorValue The recovered shared secret
     *
     * @throws std::invalid_argument If key/ciphertext parameters don't match KEM instance
     * @throws std::invalid_argument If key/ciphertext data is malformed
     *
     * @note This operation provides implicit authentication of the sender.
     * @warning Private key data should be securely erased after use.
     */
    ColorValue decapsulate(const ColorPublicKey& public_key,
                           const ColorPrivateKey& private_key,
                           const ColorCiphertext& ciphertext);

    // Key verification
    bool verify_keypair(const ColorPublicKey& public_key, const ColorPrivateKey& private_key) const;

    // Deterministic key generation (for KATs)
    std::pair<ColorPublicKey, ColorPrivateKey> keygen_deterministic(const std::array<uint8_t, 32>& matrix_seed,
                                                                   const std::array<uint8_t, 32>& secret_seed,
                                                                   const std::array<uint8_t, 32>& error_seed);

    // Deterministic encapsulation (for KATs)
    std::pair<ColorCiphertext, ColorValue> encapsulate_deterministic(const ColorPublicKey& public_key,
                                                                     const std::array<uint8_t, 32>& r_seed,
                                                                     const std::array<uint8_t, 32>& e1_seed,
                                                                     const std::array<uint8_t, 32>& e2_seed,
                                                                     const ColorValue& shared_secret);

    // Getters
    const CLWEParameters& params() const { return params_; }
};

} // namespace clwe

#endif // COLOR_KEM_HPP