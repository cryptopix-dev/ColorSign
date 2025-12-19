/**
 * @file color_ntt_engine.hpp
 * @brief Color-based Number Theoretic Transform engine for polynomial arithmetic
 *
 * This header defines the ColorNTTEngine class, which extends the base NTTEngine
 * to work with ColorValue coefficients. The NTT (Number Theoretic Transform)
 * enables fast polynomial multiplication in the ring R_q = Z_q[X]/(X^n + 1),
 * which is fundamental to lattice-based cryptographic operations.
 *
 * The ColorNTTEngine adapts standard NTT algorithms to work with color
 * coefficients while maintaining mathematical correctness and performance.
 *
 * @author ColorKEM Development Team
 * @version 1.0.0
 * @date 2024
 *
 * @see NTTEngine for the base NTT interface
 * @see ColorValue for color coefficient representation
 */

#ifndef COLOR_NTT_ENGINE_HPP
#define COLOR_NTT_ENGINE_HPP

#include "clwe/ntt_engine.hpp"
#include "clwe/color_value.hpp"
#include <vector>

namespace clwe {

/**
 * @brief Color-aware Number Theoretic Transform engine
 *
 * Extends NTTEngine to perform fast polynomial arithmetic using ColorValue
 * coefficients. This class handles the transformation between coefficient
 * and evaluation representations of polynomials, enabling efficient multiplication
 * in the ring R_q = Z_q[X]/(X^n + 1).
 *
 * Key features:
 * - Forward and inverse NTT transforms for color polynomials
 * - Fast polynomial multiplication via NTT convolution
 * - Conversion between ColorValue and uint32_t representations
 * - Modular arithmetic operations on color coefficients
 *
 * The engine precomputes NTT roots (zetas) as ColorValue objects to maintain
 * consistency with the color-based arithmetic throughout the cryptographic operations.
 */
class ColorNTTEngine : public NTTEngine {
private:
    std::vector<ColorValue> color_zetas_;
    std::vector<ColorValue> color_zetas_inv_;

    ColorValue color_to_crypto_space(const ColorValue& color) const;
    ColorValue crypto_space_to_color(const ColorValue& crypto_val) const;

    void color_butterfly(ColorValue& a, ColorValue& b, const ColorValue& zeta, uint32_t modulus) const;
    void color_butterfly_inv(ColorValue& a, ColorValue& b, const ColorValue& zeta, uint32_t modulus) const;

    ColorValue color_add_precise(const ColorValue& a, const ColorValue& b, uint32_t modulus) const;
    ColorValue color_subtract_precise(const ColorValue& a, const ColorValue& b, uint32_t modulus) const;
    ColorValue color_multiply_precise(const ColorValue& a, const ColorValue& b, uint32_t modulus) const;

public:
    /**
     * @brief Construct a ColorNTTEngine for the given parameters
     *
     * Initializes the NTT engine with the specified modulus q and ring dimension n.
     * Precomputes the NTT roots (zetas) as ColorValue objects for efficient transforms.
     *
     * @param q Prime modulus for the ring R_q
     * @param n Ring dimension (must be a power of 2)
     *
     * @throws std::invalid_argument If n is not a power of 2 or parameters are invalid
     * @throws std::runtime_error If NTT root computation fails
     */
    ColorNTTEngine(uint32_t q, uint32_t n);

    /** @brief Destructor - releases precomputed NTT roots */
    ~ColorNTTEngine() override = default;

    /**
     * @brief Forward NTT transform for color polynomials
     *
     * Transforms a polynomial from coefficient representation to evaluation
     * representation using the Number Theoretic Transform. This enables
     * fast multiplication via pointwise multiplication in the evaluation domain.
     *
     * @param poly Pointer to polynomial coefficients (modified in-place)
     *
     * @note The polynomial must have exactly n coefficients where n is the ring dimension
     */
    void ntt_forward_colors(ColorValue* poly) const;

    /**
     * @brief Inverse NTT transform for color polynomials
     *
     * Transforms a polynomial from evaluation representation back to coefficient
     * representation. Includes the necessary scaling by n^(-1) modulo q.
     *
     * @param poly Pointer to polynomial values (modified in-place)
     *
     * @note The polynomial must have exactly n values where n is the ring dimension
     */
    void ntt_inverse_colors(ColorValue* poly) const;

    /**
     * @brief Multiply two color polynomials using NTT
     *
     * Performs fast polynomial multiplication in R_q using the NTT:
     * 1. Forward NTT of both input polynomials
     * 2. Pointwise multiplication in evaluation domain
     * 3. Inverse NTT to get coefficient representation
     *
     * @param a First polynomial (n coefficients)
     * @param b Second polynomial (n coefficients)
     * @param result Output polynomial (n coefficients, overwritten)
     */
    void multiply_colors(const ColorValue* a, const ColorValue* b, ColorValue* result) const;

    // Base class interface implementations
    /**
     * @brief Forward NTT for uint32_t polynomials (base class interface)
     * @param poly Polynomial to transform (modified in-place)
     */
    void ntt_forward(uint32_t* poly) const override;

    /**
     * @brief Inverse NTT for uint32_t polynomials (base class interface)
     * @param poly Polynomial to transform (modified in-place)
     */
    void ntt_inverse(uint32_t* poly) const override;

    /**
     * @brief Multiply uint32_t polynomials using NTT (base class interface)
     * @param a First polynomial
     * @param b Second polynomial
     * @param result Output polynomial (overwritten)
     */
    void multiply(const uint32_t* a, const uint32_t* b, uint32_t* result) const override;

    /**
     * @brief Get SIMD support level (ColorNTTEngine uses scalar operations)
     * @return SIMDSupport Always returns SIMDSupport::NONE
     */
    SIMDSupport get_simd_support() const override { return SIMDSupport::NONE; }

    /**
     * @brief Convert uint32_t coefficients to ColorValue representation
     *
     * Converts an array of uint32_t coefficients to their ColorValue equivalents.
     * The conversion uses ColorValue::from_math_value() for each coefficient.
     *
     * @param coeffs Array of uint32_t coefficients (n elements)
     * @param colors Output array of ColorValue objects (must be pre-allocated, n elements)
     *
     * @note Both arrays must have exactly n elements where n is the ring dimension
     */
    void convert_uint32_to_colors(const uint32_t* coeffs, ColorValue* colors) const;

    /**
     * @brief Convert ColorValue coefficients to uint32_t representation
     *
     * Converts an array of ColorValue coefficients to their uint32_t equivalents.
     * The conversion uses ColorValue::to_math_value() for each coefficient.
     *
     * @param colors Array of ColorValue coefficients (n elements)
     * @param coeffs Output array of uint32_t values (must be pre-allocated, n elements)
     *
     * @note Both arrays must have exactly n elements where n is the ring dimension
     */
    void convert_colors_to_uint32(const ColorValue* colors, uint32_t* coeffs) const;
};

} // namespace clwe

#endif // COLOR_NTT_ENGINE_HPP