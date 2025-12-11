#include <gtest/gtest.h>
#include "parameters.hpp"
#include <stdexcept>

namespace {

// Test fixture for CLWEParameters
class CLWEParametersTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup if needed
    }
};

TEST_F(CLWEParametersTest, DefaultConstructorSecurityLevel44) {
    clwe::CLWEParameters params(44);

    EXPECT_EQ(params.security_level, 44u);
    EXPECT_EQ(params.degree, 256u);
    EXPECT_EQ(params.module_rank, 4u);
    EXPECT_EQ(params.repetitions, 4u);
    EXPECT_EQ(params.modulus, 8380417u);
    EXPECT_EQ(params.eta, 2u);
    EXPECT_EQ(params.tau, 39u);
    EXPECT_EQ(params.beta, 78u);
    EXPECT_EQ(params.gamma1, 1u << 17);
    EXPECT_EQ(params.gamma2, (8380417u - 1) / 88);
    EXPECT_EQ(params.omega, 80u);
    EXPECT_EQ(params.lambda, 128u);
}

TEST_F(CLWEParametersTest, DefaultConstructorSecurityLevel65) {
    clwe::CLWEParameters params(65);

    EXPECT_EQ(params.security_level, 65u);
    EXPECT_EQ(params.degree, 256u);
    EXPECT_EQ(params.module_rank, 6u);
    EXPECT_EQ(params.repetitions, 5u);
    EXPECT_EQ(params.modulus, 8380417u);
    EXPECT_EQ(params.eta, 4u);
    EXPECT_EQ(params.tau, 49u);
    EXPECT_EQ(params.beta, 196u);
    EXPECT_EQ(params.gamma1, 1u << 19);
    EXPECT_EQ(params.gamma2, (8380417u - 1) / 32);
    EXPECT_EQ(params.omega, 55u);
    EXPECT_EQ(params.lambda, 192u);
}

TEST_F(CLWEParametersTest, DefaultConstructorSecurityLevel87) {
    clwe::CLWEParameters params(87);

    EXPECT_EQ(params.security_level, 87u);
    EXPECT_EQ(params.degree, 256u);
    EXPECT_EQ(params.module_rank, 8u);
    EXPECT_EQ(params.repetitions, 7u);
    EXPECT_EQ(params.modulus, 8380417u);
    EXPECT_EQ(params.eta, 2u);
    EXPECT_EQ(params.tau, 60u);
    EXPECT_EQ(params.beta, 120u);
    EXPECT_EQ(params.gamma1, 1u << 19);
    EXPECT_EQ(params.gamma2, (8380417u - 1) / 32);
    EXPECT_EQ(params.omega, 75u);
    EXPECT_EQ(params.lambda, 256u);
}


TEST_F(CLWEParametersTest, ValidationValidParameters) {
    // Should not throw
    EXPECT_NO_THROW(clwe::CLWEParameters params(44));
    EXPECT_NO_THROW(clwe::CLWEParameters params(65));
    EXPECT_NO_THROW(clwe::CLWEParameters params(87));
}

TEST_F(CLWEParametersTest, ValidationInvalidSecurityLevel) {
    EXPECT_THROW(clwe::CLWEParameters(0), std::invalid_argument);
    EXPECT_THROW(clwe::CLWEParameters(1), std::invalid_argument);
    EXPECT_THROW(clwe::CLWEParameters(43), std::invalid_argument);
    EXPECT_THROW(clwe::CLWEParameters(45), std::invalid_argument);
    EXPECT_THROW(clwe::CLWEParameters(100), std::invalid_argument);
}


TEST_F(CLWEParametersTest, IsPrimeFunction) {
    // Small primes
    EXPECT_TRUE(clwe::CLWEParameters::is_prime(2));
    EXPECT_TRUE(clwe::CLWEParameters::is_prime(3));
    EXPECT_TRUE(clwe::CLWEParameters::is_prime(5));
    EXPECT_TRUE(clwe::CLWEParameters::is_prime(7));
    EXPECT_TRUE(clwe::CLWEParameters::is_prime(11));
    EXPECT_TRUE(clwe::CLWEParameters::is_prime(13));

    // Small composites
    EXPECT_FALSE(clwe::CLWEParameters::is_prime(1));
    EXPECT_FALSE(clwe::CLWEParameters::is_prime(4));
    EXPECT_FALSE(clwe::CLWEParameters::is_prime(6));
    EXPECT_FALSE(clwe::CLWEParameters::is_prime(8));
    EXPECT_FALSE(clwe::CLWEParameters::is_prime(9));
    EXPECT_FALSE(clwe::CLWEParameters::is_prime(10));

    // Larger primes
    EXPECT_TRUE(clwe::CLWEParameters::is_prime(8380417));  // ML-DSA modulus
    EXPECT_TRUE(clwe::CLWEParameters::is_prime(7681));  // Another common modulus

    // Larger composites
    EXPECT_FALSE(clwe::CLWEParameters::is_prime(3330));
    EXPECT_FALSE(clwe::CLWEParameters::is_prime(7680));
}


} // namespace