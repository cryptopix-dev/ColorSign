#include <gtest/gtest.h>
#include "sampling.hpp"
#include "shake_sampler.hpp"
#include <vector>
#include <set>
#include <algorithm>

namespace clwe {

class SamplingTest : public ::testing::Test {
protected:
    void SetUp() override {
        modulus = 3329; // ML-KEM modulus
        degree = 256;
        eta = 3;

        // Initialize sampler with fixed seed for reproducible tests
        sampler = std::make_unique<SHAKE256Sampler>();
        std::array<uint8_t, 32> seed = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                                       17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
        sampler->init(seed.data(), seed.size());
    }

    uint32_t modulus;
    uint32_t degree;
    uint32_t eta;
    std::unique_ptr<SHAKE256Sampler> sampler;
};

// Test SHAKE256Sampler initialization
TEST_F(SamplingTest, SHAKE256SamplerInit) {
    std::array<uint8_t, 32> seed = {0};
    EXPECT_NO_THROW(sampler->init(seed.data(), seed.size()));

    // Test with different seed lengths
    std::vector<uint8_t> short_seed = {1, 2, 3};
    EXPECT_NO_THROW(sampler->init(short_seed.data(), short_seed.size()));

    std::vector<uint8_t> long_seed(64, 42);
    EXPECT_NO_THROW(sampler->init(long_seed.data(), long_seed.size()));
}

// Test binomial coefficient sampling
TEST_F(SamplingTest, BinomialCoefficientSampling) {
    // Sample multiple coefficients
    std::vector<int32_t> coeffs;
    for (int i = 0; i < 1000; ++i) {
        int32_t coeff = sampler->sample_binomial_coefficient(eta);
        coeffs.push_back(coeff);

        // Binomial coefficients should be in range [-eta, eta]
        EXPECT_GE(coeff, -static_cast<int32_t>(eta));
        EXPECT_LE(coeff, static_cast<int32_t>(eta));
    }

    // Check that we get some non-zero values
    bool has_nonzero = false;
    for (int32_t coeff : coeffs) {
        if (coeff != 0) {
            has_nonzero = true;
            break;
        }
    }
    EXPECT_TRUE(has_nonzero);
}

// Test polynomial binomial sampling
TEST_F(SamplingTest, PolynomialBinomialSampling) {
    std::vector<uint32_t> coeffs(degree);

    EXPECT_NO_THROW(sampler->sample_polynomial_binomial(coeffs.data(), degree, eta, modulus));

    // Check range
    for (uint32_t coeff : coeffs) {
        EXPECT_LT(coeff, modulus);
        EXPECT_GE(coeff, 0u);
    }

    // Check that not all coefficients are zero (with high probability)
    bool has_nonzero = false;
    for (uint32_t coeff : coeffs) {
        if (coeff != 0) {
            has_nonzero = true;
            break;
        }
    }
    EXPECT_TRUE(has_nonzero);
}

// Test polynomial uniform sampling
TEST_F(SamplingTest, PolynomialUniformSampling) {
    std::vector<uint32_t> coeffs(degree);

    EXPECT_NO_THROW(sampler->sample_polynomial_uniform(coeffs.data(), degree, modulus));

    // Check range
    for (uint32_t coeff : coeffs) {
        EXPECT_LT(coeff, modulus);
        EXPECT_GE(coeff, 0u);
    }

    // For uniform sampling, we expect a good distribution
    // Check that we don't have too many zeros
    int zero_count = 0;
    for (uint32_t coeff : coeffs) {
        if (coeff == 0) zero_count++;
    }
    // With uniform sampling over a large modulus, zeros should be rare
    EXPECT_LT(zero_count, degree / 2);
}

// Test batch binomial sampling
TEST_F(SamplingTest, BatchBinomialSampling) {
    const uint32_t batch_size = 10;
    uint32_t** coeffs_batch = new uint32_t*[batch_size];
    for (uint32_t i = 0; i < batch_size; ++i) {
        coeffs_batch[i] = new uint32_t[degree];
    }

    EXPECT_NO_THROW(sampler->sample_polynomial_binomial_batch(coeffs_batch, batch_size, degree, eta, modulus));

    // Check all polynomials in batch
    for (uint32_t i = 0; i < batch_size; ++i) {
        for (uint32_t j = 0; j < degree; ++j) {
            EXPECT_LT(coeffs_batch[i][j], modulus);
            EXPECT_GE(coeffs_batch[i][j], 0u);
        }
    }

    // Cleanup
    for (uint32_t i = 0; i < batch_size; ++i) {
        delete[] coeffs_batch[i];
    }
    delete[] coeffs_batch;
}

// Test uniform sampling
TEST_F(SamplingTest, UniformSampling) {
    std::vector<uint32_t> samples;
    for (int i = 0; i < 1000; ++i) {
        uint32_t sample = sampler->sample_uniform(modulus);
        samples.push_back(sample);
        EXPECT_LT(sample, modulus);
        EXPECT_GE(sample, 0u);
    }

    // Check distribution is roughly uniform
    std::sort(samples.begin(), samples.end());
    auto unique_end = std::unique(samples.begin(), samples.end());
    size_t unique_count = unique_end - samples.begin();

    // Should have many unique values
    EXPECT_GT(unique_count, 100u);
}

// Test random bytes generation
TEST_F(SamplingTest, RandomBytes) {
    const size_t len = 64;
    std::vector<uint8_t> bytes(len);

    EXPECT_NO_THROW(sampler->random_bytes(bytes.data(), len));

    // Check that not all bytes are zero
    bool has_nonzero = false;
    for (uint8_t byte : bytes) {
        if (byte != 0) {
            has_nonzero = true;
            break;
        }
    }
    EXPECT_TRUE(has_nonzero);

    // Test different lengths
    std::vector<uint8_t> small_bytes(1);
    EXPECT_NO_THROW(sampler->random_bytes(small_bytes.data(), 1));

    std::vector<uint8_t> large_bytes(1024);
    EXPECT_NO_THROW(sampler->random_bytes(large_bytes.data(), 1024));
}

// Test global sampling functions
TEST_F(SamplingTest, GlobalSamplingFunctions) {
    std::vector<uint32_t> coeffs(degree);

    // Test binomial sampling
    EXPECT_NO_THROW(sample_polynomial_binomial(coeffs.data(), degree, eta, modulus));

    for (uint32_t coeff : coeffs) {
        EXPECT_LT(coeff, modulus);
        EXPECT_GE(coeff, 0u);
    }

    // Test batch binomial sampling
    const uint32_t batch_size = 5;
    uint32_t** coeffs_batch = new uint32_t*[batch_size];
    for (uint32_t i = 0; i < batch_size; ++i) {
        coeffs_batch[i] = new uint32_t[degree];
    }

    EXPECT_NO_THROW(sample_polynomial_binomial_batch(coeffs_batch, batch_size, degree, eta, modulus));

    for (uint32_t i = 0; i < batch_size; ++i) {
        for (uint32_t j = 0; j < degree; ++j) {
            EXPECT_LT(coeffs_batch[i][j], modulus);
            EXPECT_GE(coeffs_batch[i][j], 0u);
        }
    }

    // Cleanup
    for (uint32_t i = 0; i < batch_size; ++i) {
        delete[] coeffs_batch[i];
    }
    delete[] coeffs_batch;
}

// Test SHAKE128Sampler
TEST_F(SamplingTest, SHAKE128Sampler) {
    SHAKE128Sampler shake128;
    std::array<uint8_t, 32> seed = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                                   17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};

    EXPECT_NO_THROW(shake128.init(seed.data(), seed.size()));

    std::vector<uint8_t> output(64);
    EXPECT_NO_THROW(shake128.squeeze(output.data(), output.size()));

    // Check output is not all zeros
    bool has_nonzero = false;
    for (uint8_t byte : output) {
        if (byte != 0) {
            has_nonzero = true;
            break;
        }
    }
    EXPECT_TRUE(has_nonzero);
}

// Test reproducibility with same seed
TEST_F(SamplingTest, Reproducibility) {
    std::array<uint8_t, 32> seed = {42};

    SHAKE256Sampler sampler1;
    sampler1.init(seed.data(), seed.size());

    SHAKE256Sampler sampler2;
    sampler2.init(seed.data(), seed.size());

    // Sample same values
    uint32_t val1 = sampler1.sample_uniform(100);
    uint32_t val2 = sampler2.sample_uniform(100);

    EXPECT_EQ(val1, val2);

    // Sample polynomials
    std::vector<uint32_t> poly1(degree);
    std::vector<uint32_t> poly2(degree);

    sampler1.sample_polynomial_binomial(poly1.data(), degree, eta, modulus);
    sampler2.sample_polynomial_binomial(poly2.data(), degree, eta, modulus);

    EXPECT_EQ(poly1, poly2);
}

// Test different eta values
TEST_F(SamplingTest, DifferentEtaValues) {
    std::vector<uint32_t> test_eta = {2, 3, 4, 5};

    for (uint32_t e : test_eta) {
        std::vector<uint32_t> coeffs(degree);
        EXPECT_NO_THROW(sampler->sample_polynomial_binomial(coeffs.data(), degree, e, modulus));

        // Check range
        for (uint32_t coeff : coeffs) {
            EXPECT_LT(coeff, modulus);
            EXPECT_GE(coeff, 0u);
        }
    }
}

// Test statistical properties (basic)
TEST_F(SamplingTest, StatisticalProperties) {
    const int num_samples = 10000;
    std::vector<uint32_t> samples;

    // Sample many uniform values
    for (int i = 0; i < num_samples; ++i) {
        samples.push_back(sampler->sample_uniform(modulus));
    }

    // Calculate mean
    double sum = 0.0;
    for (uint32_t sample : samples) {
        sum += sample;
    }
    double mean = sum / num_samples;

    // For uniform [0, modulus), mean should be around modulus/2
    double expected_mean = (modulus - 1.0) / 2.0;
    EXPECT_NEAR(mean, expected_mean, modulus * 0.1); // Allow 10% deviation
}

// Test edge cases
TEST_F(SamplingTest, EdgeCases) {
    // Test with eta = 0 (should give all zeros)
    std::vector<uint32_t> coeffs(degree);
    sampler->sample_polynomial_binomial(coeffs.data(), degree, 0, modulus);

    for (uint32_t coeff : coeffs) {
        EXPECT_EQ(coeff, 0u);
    }

    // Test with modulus = 2
    std::vector<uint32_t> small_coeffs(degree);
    sampler->sample_polynomial_binomial(small_coeffs.data(), degree, eta, 2);

    for (uint32_t coeff : small_coeffs) {
        EXPECT_LT(coeff, 2u);
        EXPECT_GE(coeff, 0u);
    }
}

#ifdef HAVE_AVX512
// Test AVX-512 batch sampling
TEST_F(SamplingTest, AVX512BatchSampling) {
    const uint32_t batch_size = 16; // AVX-512 can handle 16 polynomials
    uint32_t** coeffs_batch = new uint32_t*[batch_size];
    for (uint32_t i = 0; i < batch_size; ++i) {
        coeffs_batch[i] = new uint32_t[degree];
    }

    EXPECT_NO_THROW(sampler->sample_polynomial_binomial_batch_avx512(coeffs_batch, batch_size, degree, eta, modulus));

    // Check all polynomials
    for (uint32_t i = 0; i < batch_size; ++i) {
        for (uint32_t j = 0; j < degree; ++j) {
            EXPECT_LT(coeffs_batch[i][j], modulus);
            EXPECT_GE(coeffs_batch[i][j], 0u);
        }
    }

    // Cleanup
    for (uint32_t i = 0; i < batch_size; ++i) {
        delete[] coeffs_batch[i];
    }
    delete[] coeffs_batch;
}
#endif

} // namespace clwe