#include <gtest/gtest.h>
#include "utils.hpp"
#include <vector>
#include <chrono>
#include <thread>

namespace clwe {

class UtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common test values
        modulus = 3329; // ML-KEM modulus
    }

    uint32_t modulus;
};

// Test timestamp functions
TEST_F(UtilsTest, TimestampFunctions) {
    uint64_t ts1 = get_timestamp_ns();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    uint64_t ts2 = get_timestamp_ns();

    EXPECT_GT(ts2, ts1);

    double ms1 = timestamp_to_ms(ts1);
    double ms2 = timestamp_to_ms(ts2);

    EXPECT_GT(ms2, ms1);
    EXPECT_NEAR(ms2 - ms1, 1.0, 0.5); // Approximately 1ms difference
}

// Test Montgomery reduction
TEST_F(UtilsTest, MontgomeryReduction) {
    // Test basic reduction
    uint64_t a = 123456789ULL;
    uint32_t result = montgomery_reduce(a, modulus);
    EXPECT_LT(result, modulus);
    EXPECT_GE(result, 0u);

    // Test with multiple of modulus
    uint64_t multiple = static_cast<uint64_t>(modulus) * 42;
    result = montgomery_reduce(multiple, modulus);
    EXPECT_EQ(result, 0u);

    // Test edge cases
    EXPECT_EQ(montgomery_reduce(0, modulus), 0u);
    EXPECT_EQ(montgomery_reduce(modulus - 1, modulus), modulus - 1);
}

// Test Barrett reduction
TEST_F(UtilsTest, BarrettReduction) {
    uint64_t mu = (1ULL << 32) / modulus; // Precomputed mu for Barrett

    // Test basic reduction
    uint64_t a = 123456789ULL;
    uint32_t result = barrett_reduce(a, modulus, mu);
    EXPECT_LT(result, modulus);
    EXPECT_GE(result, 0u);

    // Test with multiple of modulus
    uint64_t multiple = static_cast<uint64_t>(modulus) * 42;
    result = barrett_reduce(multiple, modulus, mu);
    EXPECT_EQ(result, 0u);

    // Test edge cases
    EXPECT_EQ(barrett_reduce(0, modulus, mu), 0u);
    EXPECT_EQ(barrett_reduce(modulus - 1, modulus, mu), modulus - 1);
}

// Test bit operations
TEST_F(UtilsTest, BitOperations) {
    // Test bit_length
    EXPECT_EQ(bit_length(1), 1);
    EXPECT_EQ(bit_length(2), 2);
    EXPECT_EQ(bit_length(3), 2);
    EXPECT_EQ(bit_length(4), 3);
    EXPECT_EQ(bit_length(255), 8);
    EXPECT_EQ(bit_length(256), 9);
    EXPECT_EQ(bit_length(0), 0); // Edge case

    // Test is_power_of_two
    EXPECT_TRUE(is_power_of_two(1));
    EXPECT_TRUE(is_power_of_two(2));
    EXPECT_TRUE(is_power_of_two(4));
    EXPECT_TRUE(is_power_of_two(8));
    EXPECT_TRUE(is_power_of_two(256));
    EXPECT_TRUE(is_power_of_two(1024));

    EXPECT_FALSE(is_power_of_two(0));
    EXPECT_FALSE(is_power_of_two(3));
    EXPECT_FALSE(is_power_of_two(6));
    EXPECT_FALSE(is_power_of_two(255));

    // Test next_power_of_two
    EXPECT_EQ(next_power_of_two(1), 1);
    EXPECT_EQ(next_power_of_two(2), 2);
    EXPECT_EQ(next_power_of_two(3), 4);
    EXPECT_EQ(next_power_of_two(4), 4);
    EXPECT_EQ(next_power_of_two(5), 8);
    EXPECT_EQ(next_power_of_two(255), 256);
    EXPECT_EQ(next_power_of_two(257), 512);
}

// Test modular inverse
TEST_F(UtilsTest, ModularInverse) {
    // Test with prime modulus
    EXPECT_EQ(mod_inverse(1, modulus), 1u);
    EXPECT_EQ(mod_inverse(modulus - 1, modulus), modulus - 1);

    // Test Fermat's little theorem: a^(p-1) ≡ 1 mod p for p prime, a not multiple of p
    uint32_t a = 123;
    uint32_t inv = mod_inverse(a, modulus);
    uint32_t product = (static_cast<uint64_t>(a) * inv) % modulus;
    EXPECT_EQ(product, 1u);

    // Test with different values
    for (uint32_t i = 1; i < 10; ++i) {
        uint32_t inv_i = mod_inverse(i, modulus);
        uint32_t product_i = (static_cast<uint64_t>(i) * inv_i) % modulus;
        EXPECT_EQ(product_i, 1u);
    }
}

// Test modular exponentiation
TEST_F(UtilsTest, ModularExponentiation) {
    // Test basic cases
    EXPECT_EQ(mod_pow(2, 0, modulus), 1u);
    EXPECT_EQ(mod_pow(2, 1, modulus), 2u);
    EXPECT_EQ(mod_pow(2, 2, modulus), 4u);
    EXPECT_EQ(mod_pow(2, 3, modulus), 8u);

    // Test with modulus
    EXPECT_EQ(mod_pow(2, 10, 1024), 0u); // 2^10 = 1024 ≡ 0 mod 1024

    // Test Fermat's little theorem
    uint32_t a = 123;
    uint32_t result = mod_pow(a, modulus - 1, modulus);
    EXPECT_EQ(result, 1u);

    // Test edge cases
    EXPECT_EQ(mod_pow(0, 1, modulus), 0u);
    EXPECT_EQ(mod_pow(1, 100, modulus), 1u);
}

// Test secure random bytes
TEST_F(UtilsTest, SecureRandomBytes) {
    const size_t len = 32;
    uint8_t buffer[len];

    // Should not crash
    EXPECT_NO_THROW(secure_random_bytes(buffer, len));

    // Check that buffer is filled (not all zeros with high probability)
    bool all_zero = true;
    for (size_t i = 0; i < len; ++i) {
        if (buffer[i] != 0) {
            all_zero = false;
            break;
        }
    }
    EXPECT_FALSE(all_zero) << "Random bytes are all zero - unlikely";

    // Test different lengths
    uint8_t small_buffer[1];
    EXPECT_NO_THROW(secure_random_bytes(small_buffer, 1));

    uint8_t large_buffer[1024];
    EXPECT_NO_THROW(secure_random_bytes(large_buffer, 1024));
}

// Test AVXAllocator
TEST_F(UtilsTest, AVXAllocator) {
    // Test allocation and deallocation
    void* ptr = AVXAllocator::allocate(1024);
    EXPECT_NE(ptr, nullptr);

    // Test reallocation
    void* new_ptr = AVXAllocator::reallocate(ptr, 2048);
    EXPECT_NE(new_ptr, nullptr);

    AVXAllocator::deallocate(new_ptr);

    // Test edge cases
    ptr = AVXAllocator::allocate(0);
    EXPECT_NE(ptr, nullptr); // May return non-null for 0 size
    AVXAllocator::deallocate(ptr);
}

// Test AVXVector
TEST_F(UtilsTest, AVXVector) {
    // Test default construction
    AVXVector<uint32_t> vec1;
    EXPECT_EQ(vec1.size(), 0u);
    EXPECT_EQ(vec1.capacity(), 0u);
    EXPECT_TRUE(vec1.empty());

    // Test construction with capacity
    AVXVector<uint32_t> vec2(16);
    EXPECT_EQ(vec2.size(), 0u);
    EXPECT_GE(vec2.capacity(), 16u);

    // Test push_back
    vec2.push_back(42);
    EXPECT_EQ(vec2.size(), 1u);
    EXPECT_EQ(vec2[0], 42u);

    // Test multiple push_back
    for (uint32_t i = 0; i < 10; ++i) {
        vec2.push_back(i * 10);
    }
    EXPECT_EQ(vec2.size(), 11u);

    // Test resize
    vec2.resize(5);
    EXPECT_EQ(vec2.size(), 5u);

    // Test clear
    vec2.clear();
    EXPECT_EQ(vec2.size(), 0u);
    EXPECT_TRUE(vec2.empty());

    // Test reserve
    vec2.reserve(100);
    EXPECT_GE(vec2.capacity(), 100u);
}

// Test AVXVector move operations
TEST_F(UtilsTest, AVXVectorMove) {
    AVXVector<uint32_t> vec1;
    vec1.push_back(1);
    vec1.push_back(2);
    vec1.push_back(3);

    // Test move constructor
    AVXVector<uint32_t> vec2(std::move(vec1));
    EXPECT_EQ(vec2.size(), 3u);
    EXPECT_EQ(vec1.size(), 0u); // vec1 should be empty after move

    // Test move assignment
    AVXVector<uint32_t> vec3;
    vec3 = std::move(vec2);
    EXPECT_EQ(vec3.size(), 3u);
    EXPECT_EQ(vec2.size(), 0u); // vec2 should be empty after move
}

// Test AVXVector with different types
TEST_F(UtilsTest, AVXVectorTypes) {
    AVXVector<int> int_vec;
    int_vec.push_back(-42);
    EXPECT_EQ(int_vec[0], -42);

    AVXVector<double> double_vec;
    double_vec.push_back(3.14159);
    EXPECT_DOUBLE_EQ(double_vec[0], 3.14159);
}

// Test AVXVector bounds
TEST_F(UtilsTest, AVXVectorBounds) {
    AVXVector<uint32_t> vec;
    vec.push_back(1);
    vec.push_back(2);

    EXPECT_EQ(vec[0], 1u);
    EXPECT_EQ(vec[1], 2u);

    // Test const access
    const auto& const_vec = vec;
    EXPECT_EQ(const_vec[0], 1u);
    EXPECT_EQ(const_vec[1], 2u);
}

#ifdef HAVE_AVX2
// Test AVX-specific functions
TEST_F(UtilsTest, AVXFunctions) {
    avx_type vec = _mm256_set1_epi32(42);
    uint32_t result = montgomery_reduce_avx(vec, modulus);
    EXPECT_LT(result, modulus);
    EXPECT_GE(result, 0u);
}
#endif

} // namespace clwe