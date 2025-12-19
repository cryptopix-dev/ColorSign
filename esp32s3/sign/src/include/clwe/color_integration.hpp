#ifndef CLWE_COLOR_INTEGRATION_HPP
#define CLWE_COLOR_INTEGRATION_HPP

#include <vector>
#include <cstdint>

namespace clwe {

/**
 * @brief Color integration module for ColorSign
 *
 * Provides functions to encode polynomials and vectors into RGB pixel arrays
 * for visualization, and decode back. Coefficients are packed into RGB pixels
 * where each pixel represents three coefficients (R, G, B channels).
 */

/**
 * @brief Encode a single polynomial into RGB color data
 *
 * Coefficients are packed into RGB pixels where each pixel contains 3 coefficients
 * (R, G, B channels). Each coefficient is reduced modulo modulus and takes lower 8 bits.
 *
 * @param poly The polynomial coefficients
 * @param modulus The modulus for coefficient reduction (typically q)
 * @return RGB color data as byte array (3 bytes per pixel)
 */
std::vector<uint8_t> encode_polynomial_as_colors(const std::vector<uint32_t>& poly, uint32_t modulus);

/**
 * @brief Decode RGB color data into a single polynomial
 *
 * Unpacks RGB pixel data back into coefficients, where each pixel contains 3 coefficients
 * (R, G, B channels), and reduces modulo modulus.
 *
 * @param color_data RGB color data
 * @param modulus The modulus for coefficient reduction
 * @return Polynomial coefficients
 */
std::vector<uint32_t> decode_colors_to_polynomial(const std::vector<uint8_t>& color_data, uint32_t modulus);

/**
 * @brief Encode a vector of polynomials into RGB color data
 *
 * Polynomials are flattened and coefficients are packed into RGB pixels,
 * with each pixel containing 3 coefficients (R, G, B channels).
 *
 * @param poly_vector Vector of polynomials
 * @param modulus The modulus for coefficient reduction
 * @return RGB color data as byte array
 */
std::vector<uint8_t> encode_polynomial_vector_as_colors(const std::vector<std::vector<uint32_t>>& poly_vector, uint32_t modulus);

/**
 * @brief Decode RGB color data into a vector of polynomials
 *
 * @param color_data RGB color data where each pixel contains 3 coefficients
 * @param k Number of polynomials in the vector
 * @param n Degree of each polynomial
 * @param modulus The modulus for coefficient reduction
 * @return Vector of polynomials
 */
std::vector<std::vector<uint32_t>> decode_colors_to_polynomial_vector(const std::vector<uint8_t>& color_data, uint32_t k, uint32_t n, uint32_t modulus);

// Compression functions for color integration
std::vector<uint8_t> encode_polynomial_vector_as_colors_compressed(const std::vector<std::vector<uint32_t>>& poly_vector, uint32_t modulus);
std::vector<uint8_t> encode_polynomial_vector_as_colors_huffman(const std::vector<std::vector<uint32_t>>& poly_vector, uint32_t modulus);
std::vector<std::vector<uint32_t>> decode_colors_to_polynomial_vector_compressed(const std::vector<uint8_t>& color_data, uint32_t k, uint32_t n, uint32_t modulus);
std::vector<uint8_t> convert_compressed_to_color_format(const std::vector<uint8_t>& compressed_data, uint32_t k, uint32_t n, uint32_t modulus);
std::vector<uint8_t> encode_polynomial_vector_as_colors_auto(const std::vector<std::vector<uint32_t>>& poly_vector, uint32_t modulus);

// Advanced color integration functions
std::vector<uint8_t> generate_color_representation_from_compressed(const std::vector<uint8_t>& compressed_data, uint32_t k, uint32_t n, uint32_t modulus);
std::vector<uint8_t> compress_with_color_support(const std::vector<std::vector<uint32_t>>& poly_vector, uint32_t modulus, bool enable_color_metadata = true);
std::vector<std::vector<uint32_t>> decompress_with_color_support(const std::vector<uint8_t>& dual_format_data, uint32_t& out_k, uint32_t& out_n, uint32_t& out_modulus);
std::vector<uint8_t> generate_color_from_dual_format(const std::vector<uint8_t>& dual_format_data);
std::vector<uint8_t> encode_polynomial_vector_with_color_integration(const std::vector<std::vector<uint32_t>>& poly_vector, uint32_t modulus, bool enable_on_demand_color = true);
std::vector<std::vector<uint32_t>> decode_polynomial_vector_with_color_integration(const std::vector<uint8_t>& color_integrated_data, uint32_t modulus);

// KEM key specific color encoding functions
std::vector<uint8_t> encode_color_kem_key_as_colors_compressed(const std::vector<uint8_t>& key_data);
std::vector<uint8_t> decode_colors_to_color_kem_key_compressed(const std::vector<uint8_t>& color_data, size_t expected_size);

} // namespace clwe

#endif // CLWE_COLOR_INTEGRATION_HPP