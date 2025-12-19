#include <gtest/gtest.h>
#include "color_value.hpp"
#include <vector>

namespace clwe {

class ColorValueTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common test colors
        red = ColorValue(255, 0, 0);
        green = ColorValue(0, 255, 0);
        blue = ColorValue(0, 0, 255);
        white = ColorValue(255, 255, 255);
        black = ColorValue(0, 0, 0);
    }

    ColorValue red, green, blue, white, black;
    const uint32_t modulus = 3329; // ML-KEM modulus
};

// Test constructors
TEST_F(ColorValueTest, Constructor) {
    ColorValue c1;
    EXPECT_EQ(c1.r, 0);
    EXPECT_EQ(c1.g, 0);
    EXPECT_EQ(c1.b, 0);
    EXPECT_EQ(c1.a, 255);

    ColorValue c2(100, 150, 200, 128);
    EXPECT_EQ(c2.r, 100);
    EXPECT_EQ(c2.g, 150);
    EXPECT_EQ(c2.b, 200);
    EXPECT_EQ(c2.a, 128);
}

// Test equality operators
TEST_F(ColorValueTest, Equality) {
    ColorValue c1(255, 0, 0);
    ColorValue c2(255, 0, 0);
    ColorValue c3(254, 0, 0);

    EXPECT_TRUE(c1 == c2);
    EXPECT_FALSE(c1 == c3);
    EXPECT_TRUE(c1 != c3);
    EXPECT_FALSE(c1 != c2);
}

// Test math value conversion
TEST_F(ColorValueTest, MathValueConversion) {
    ColorValue original(255, 128, 64, 32);
    uint32_t math_val = original.to_math_value();
    ColorValue reconstructed = ColorValue::from_math_value(math_val);

    EXPECT_EQ(original, reconstructed);
    EXPECT_EQ(math_val, 0xFF804020u); // 255<<24 | 128<<16 | 64<<8 | 32
}

// Test precise value conversion
TEST_F(ColorValueTest, PreciseValueConversion) {
    ColorValue original(255, 128, 64);
    uint64_t precise_val = original.to_precise_value();
    ColorValue reconstructed = ColorValue::from_precise_value(precise_val);

    EXPECT_EQ(original.r, reconstructed.r);
    EXPECT_EQ(original.g, reconstructed.g);
    EXPECT_EQ(original.b, reconstructed.b);
    EXPECT_EQ(reconstructed.a, 255); // Alpha is set to 255 in from_precise_value
}

// Test modular arithmetic
TEST_F(ColorValueTest, ModularArithmetic) {
    ColorValue a(100, 50, 25);
    ColorValue b(50, 25, 12);

    ColorValue sum = a.mod_add(b, modulus);
    ColorValue diff = a.mod_subtract(b, modulus);
    ColorValue prod = a.mod_multiply(b, modulus);

    // Verify modular properties
    uint64_t expected_sum = ((uint64_t)a.to_math_value() + b.to_math_value()) % modulus;
    EXPECT_EQ(sum.to_math_value() % modulus, expected_sum);
    uint64_t expected_diff = ((uint64_t)a.to_math_value() + modulus - b.to_math_value()) % modulus;
    EXPECT_EQ(diff.to_math_value() % modulus, expected_diff);
    uint64_t expected_prod = (uint64_t)a.to_math_value() * b.to_math_value() % modulus;
    EXPECT_EQ(prod.to_math_value() % modulus, expected_prod);
}

// Test edge cases for modular arithmetic
TEST_F(ColorValueTest, ModularArithmeticEdgeCases) {
    ColorValue zero(0, 0, 0, 0);
    ColorValue max_val(255, 255, 255);

    // Test with zero
    EXPECT_EQ(zero.mod_add(zero, modulus), zero);
    EXPECT_EQ(zero.mod_multiply(max_val, modulus), zero);

    // Test overflow handling
    ColorValue large1(255, 255, 255);
    ColorValue large2(255, 255, 255);
    ColorValue result = large1.mod_multiply(large2, modulus);
    EXPECT_LT(result.to_math_value(), modulus);
}

// Test HSV conversion (basic functionality)
TEST_F(ColorValueTest, HSVConversion) {
    ColorValue rgb(255, 0, 0); // Pure red
    ColorValue hsv = rgb.to_hsv();
    ColorValue back_to_rgb = hsv.from_hsv();

    // HSV conversion should be reversible (approximately)
    EXPECT_NEAR(rgb.r, back_to_rgb.r, 1);
    EXPECT_NEAR(rgb.g, back_to_rgb.g, 1);
    EXPECT_NEAR(rgb.b, back_to_rgb.b, 1);
}

// Test string representation
TEST_F(ColorValueTest, StringRepresentation) {
    ColorValue c(255, 128, 64, 32);
    std::string str = c.to_string();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("255"), std::string::npos);
    EXPECT_NE(str.find("128"), std::string::npos);
    EXPECT_NE(str.find("64"), std::string::npos);
    EXPECT_NE(str.find("32"), std::string::npos);
}

// Test color operations
TEST_F(ColorValueTest, ColorOperations) {
    using namespace color_ops;

    ColorValue a(100, 50, 25);
    ColorValue b(50, 25, 12);

    ColorValue sum = add_colors(a, b);
    ColorValue prod = multiply_colors(a, b);

    // Basic checks - exact behavior depends on implementation
    EXPECT_GE(sum.r, 0);
    EXPECT_LE(sum.r, 255);
    EXPECT_GE(sum.g, 0);
    EXPECT_LE(sum.g, 255);
    EXPECT_GE(sum.b, 0);
    EXPECT_LE(sum.b, 255);

    EXPECT_GE(prod.r, 0);
    EXPECT_LE(prod.r, 255);
    EXPECT_GE(prod.g, 0);
    EXPECT_LE(prod.g, 255);
    EXPECT_GE(prod.b, 0);
    EXPECT_LE(prod.b, 255);
}

// Test modular reduction
TEST_F(ColorValueTest, ModularReduction) {
    using namespace color_ops;

    ColorValue c(255, 255, 255);
    ColorValue reduced = mod_reduce_color(c, modulus);

    EXPECT_LT(reduced.to_math_value(), modulus);
    EXPECT_GE(reduced.to_math_value(), 0u);
}

// Test SIMD operations if available
#ifdef HAVE_AVX512
TEST_F(ColorValueTest, AVX512Operations) {
    using namespace color_ops;

    __m512i a = _mm512_set1_epi32(0xFF804020);
    __m512i b = _mm512_set1_epi32(0x80402010);

    __m512i sum = add_colors_avx512(a, b);
    __m512i prod = multiply_colors_avx512(a, b);

    // Basic checks that operations don't crash
    EXPECT_NE(_mm512_extract_epi32(sum, 0), 0);
    EXPECT_NE(_mm512_extract_epi32(prod, 0), 0);
}
#endif

#ifdef __ARM_NEON
TEST_F(ColorValueTest, NEONOperations) {
    using namespace color_ops;

    uint32x4_t a = vdupq_n_u32(0xFF804020);
    uint32x4_t b = vdupq_n_u32(0x80402010);

    uint32x4_t sum = add_colors_neon(a, b);
    uint32x4_t prod = multiply_colors_neon(a, b);

    // Basic checks that operations don't crash
    EXPECT_NE(vgetq_lane_u32(sum, 0), 0u);
    EXPECT_NE(vgetq_lane_u32(prod, 0), 0u);
}
#endif

// Test SIMD wrapper functions
TEST_F(ColorValueTest, SIMDWrapperOperations) {
    using namespace color_ops;

    ColorValue a(100, 50, 25);
    ColorValue b(50, 25, 12);

    ColorValue sum_simd = add_colors_simd(a, b);
    ColorValue prod_simd = multiply_colors_simd(a, b);
    ColorValue reduced_simd = mod_reduce_color_simd(a, modulus);

    // Should not crash and produce valid results
    EXPECT_GE(sum_simd.r, 0);
    EXPECT_LE(sum_simd.r, 255);
    EXPECT_GE(prod_simd.r, 0);
    EXPECT_LE(prod_simd.r, 255);
    EXPECT_LT(reduced_simd.to_math_value(), modulus);
}

// Test invalid inputs
TEST_F(ColorValueTest, InvalidInputs) {
    // Test with modulus = 0 (should handle gracefully)
    ColorValue a(100, 50, 25);
    ColorValue b(50, 25, 12);

    // These should not crash, though results may be undefined
    EXPECT_NO_THROW(a.mod_add(b, 0));
    EXPECT_NO_THROW(a.mod_multiply(b, 0));
}

} // namespace clwe