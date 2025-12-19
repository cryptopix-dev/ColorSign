#include <gtest/gtest.h>
#include "clwe.hpp"
#include <stdexcept>

namespace clwe {

class CLWEParametersTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Valid parameters for different security levels
        params512 = CLWEParameters(512);
        params768 = CLWEParameters(768);
        params1024 = CLWEParameters(1024);
    }

    CLWEParameters params512, params768, params1024;
};

// Test default constructor with security levels
TEST_F(CLWEParametersTest, ConstructorWithSecurityLevel) {
    EXPECT_EQ(params512.security_level, 512);
    EXPECT_EQ(params512.degree, 256);
    EXPECT_EQ(params512.module_rank, 2);
    EXPECT_EQ(params512.modulus, 3329);
    EXPECT_EQ(params512.eta1, 3);
    EXPECT_EQ(params512.eta2, 2);

    EXPECT_EQ(params768.security_level, 768);
    EXPECT_EQ(params768.degree, 256);
    EXPECT_EQ(params768.module_rank, 3);
    EXPECT_EQ(params768.modulus, 3329);
    EXPECT_EQ(params768.eta1, 2);
    EXPECT_EQ(params768.eta2, 2);

    EXPECT_EQ(params1024.security_level, 1024);
    EXPECT_EQ(params1024.degree, 256);
    EXPECT_EQ(params1024.module_rank, 4);
    EXPECT_EQ(params1024.modulus, 3329);
    EXPECT_EQ(params1024.eta1, 2);
    EXPECT_EQ(params1024.eta2, 2);
}

// Test custom constructor
TEST_F(CLWEParametersTest, CustomConstructor) {
    CLWEParameters custom(512, 256, 2, 3329, 3, 2);
    EXPECT_EQ(custom.security_level, 512);
    EXPECT_EQ(custom.degree, 256);
    EXPECT_EQ(custom.module_rank, 2);
    EXPECT_EQ(custom.modulus, 3329);
    EXPECT_EQ(custom.eta1, 3);
    EXPECT_EQ(custom.eta2, 2);
}

// Test validation - valid parameters
TEST_F(CLWEParametersTest, ValidParameters) {
    EXPECT_NO_THROW(params512.validate());
    EXPECT_NO_THROW(params768.validate());
    EXPECT_NO_THROW(params1024.validate());

    // Test custom valid parameters
    CLWEParameters custom(512, 512, 2, 7681, 2, 2); // Larger prime
    EXPECT_NO_THROW(custom.validate());
}

// Test validation - invalid security level
TEST_F(CLWEParametersTest, InvalidSecurityLevel) {
    EXPECT_THROW(CLWEParameters(256), std::invalid_argument);
    EXPECT_THROW(CLWEParameters(2048), std::invalid_argument);
    EXPECT_THROW(CLWEParameters(0), std::invalid_argument);
}

// Test validation - invalid degree
TEST_F(CLWEParametersTest, InvalidDegree) {
    EXPECT_THROW(CLWEParameters(512, 0, 2, 3329, 3, 2), std::invalid_argument);
    EXPECT_THROW(CLWEParameters(512, 100, 2, 3329, 3, 2), std::invalid_argument); // Not power of 2
    EXPECT_THROW(CLWEParameters(512, 8193, 2, 3329, 3, 2), std::invalid_argument); // Too large
    EXPECT_THROW(CLWEParameters(512, 3, 2, 3329, 3, 2), std::invalid_argument); // Not power of 2
}

// Test validation - invalid module rank
TEST_F(CLWEParametersTest, InvalidModuleRank) {
    EXPECT_THROW(CLWEParameters(512, 256, 0, 3329, 3, 2), std::invalid_argument);
    EXPECT_THROW(CLWEParameters(512, 256, 17, 3329, 3, 2), std::invalid_argument);
}

// Test validation - invalid modulus
TEST_F(CLWEParametersTest, InvalidModulus) {
    EXPECT_THROW(CLWEParameters(512, 256, 2, 256, 3, 2), std::invalid_argument); // Too small
    EXPECT_THROW(CLWEParameters(512, 256, 2, 65537, 3, 2), std::invalid_argument); // Too large
    EXPECT_THROW(CLWEParameters(512, 256, 2, 4, 3, 2), std::invalid_argument); // Not prime
    EXPECT_THROW(CLWEParameters(512, 256, 2, 9, 3, 2), std::invalid_argument); // Not prime
    EXPECT_THROW(CLWEParameters(512, 256, 2, 15, 3, 2), std::invalid_argument); // Not prime
}

// Test validation - invalid eta values
TEST_F(CLWEParametersTest, InvalidEtaValues) {
    EXPECT_THROW(CLWEParameters(512, 256, 2, 3329, 0, 2), std::invalid_argument);
    EXPECT_THROW(CLWEParameters(512, 256, 2, 3329, 17, 2), std::invalid_argument);
    EXPECT_THROW(CLWEParameters(512, 256, 2, 3329, 3, 0), std::invalid_argument);
    EXPECT_THROW(CLWEParameters(512, 256, 2, 3329, 3, 17), std::invalid_argument);
}

// Test prime checking helper
TEST_F(CLWEParametersTest, PrimeChecking) {
    // Test known primes
    EXPECT_TRUE(CLWEParameters::is_prime(2));
    EXPECT_TRUE(CLWEParameters::is_prime(3));
    EXPECT_TRUE(CLWEParameters::is_prime(5));
    EXPECT_TRUE(CLWEParameters::is_prime(7));
    EXPECT_TRUE(CLWEParameters::is_prime(11));
    EXPECT_TRUE(CLWEParameters::is_prime(13));
    EXPECT_TRUE(CLWEParameters::is_prime(3329)); // ML-KEM modulus

    // Test known composites
    EXPECT_FALSE(CLWEParameters::is_prime(1));
    EXPECT_FALSE(CLWEParameters::is_prime(4));
    EXPECT_FALSE(CLWEParameters::is_prime(6));
    EXPECT_FALSE(CLWEParameters::is_prime(8));
    EXPECT_FALSE(CLWEParameters::is_prime(9));
    EXPECT_FALSE(CLWEParameters::is_prime(10));
    EXPECT_FALSE(CLWEParameters::is_prime(15));
}

// Test edge cases for prime checking
TEST_F(CLWEParametersTest, PrimeEdgeCases) {
    EXPECT_FALSE(CLWEParameters::is_prime(0));
    EXPECT_FALSE(CLWEParameters::is_prime(1));
    EXPECT_TRUE(CLWEParameters::is_prime(2));
    EXPECT_TRUE(CLWEParameters::is_prime(3));

    // Test larger primes
    EXPECT_TRUE(CLWEParameters::is_prime(7681)); // Another ML-KEM modulus
    EXPECT_TRUE(CLWEParameters::is_prime(12289)); // Dilithium modulus
}

// Test parameter combinations that should work
TEST_F(CLWEParametersTest, ValidParameterCombinations) {
    // Test various valid combinations
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t>> valid_params = {
        {512, 256, 2, 3329, 3, 2},
        {512, 512, 2, 7681, 2, 2},
        {768, 256, 3, 3329, 2, 2},
        {1024, 256, 4, 3329, 2, 2},
        {512, 1024, 2, 12289, 2, 2}
    };

    for (auto& params : valid_params) {
        uint32_t sec, deg, rank, mod, e1, e2;
        std::tie(sec, deg, rank, mod, e1, e2) = params;
        EXPECT_NO_THROW(CLWEParameters(sec, deg, rank, mod, e1, e2));
    }
}

// Test that validation is called in constructors
TEST_F(CLWEParametersTest, ValidationInConstructor) {
    EXPECT_THROW(CLWEParameters(999, 256, 2, 3329, 3, 2), std::invalid_argument);
    EXPECT_THROW(CLWEParameters(512, 256, 2, 3330, 3, 2), std::invalid_argument); // 3330 not prime
}

// Test copy behavior
TEST_F(CLWEParametersTest, CopyBehavior) {
    CLWEParameters original(512);
    CLWEParameters copy = original;

    EXPECT_EQ(copy.security_level, original.security_level);
    EXPECT_EQ(copy.degree, original.degree);
    EXPECT_EQ(copy.module_rank, original.module_rank);
    EXPECT_EQ(copy.modulus, original.modulus);
    EXPECT_EQ(copy.eta1, original.eta1);
    EXPECT_EQ(copy.eta2, original.eta2);
}

} // namespace clwe