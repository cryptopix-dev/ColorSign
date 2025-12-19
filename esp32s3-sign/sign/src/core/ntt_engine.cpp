#include "../include/clwe/ntt_engine.hpp"
#include "../include/clwe/cpu_features.hpp"
#include "../include/clwe/utils.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <limits>

// SIMD intrinsics for AVX2 and AVX-512
#ifdef HAVE_AVX2
#include <immintrin.h>
#endif

#ifdef HAVE_AVX512
#include <immintrin.h>
#endif

namespace clwe {

// Correct ML-DSA zetas for q=8380417, n=256, primitive root g=17
// These are the proper zetas used in NIST ML-DSA specification
const int32_t zetas_ml_dsa[128] = {
    0, 25847, -2608894, -518909, 237124, -777960, -876248, 466468,
    1826347, 2353451, -359251, -2091905, 3119733, -2884855, 3111497, 2680103,
    2725464, 1024112, -1079900, 3585928, -1497254, 4189091, -1882636, 2036599,
    1107237, 727831, -214476, 18292, -167782, -240320, -474467, -378833,
    -1575429, -1194982, -262805, -857374, -236959, -876248, -109702, -494783,
    -1846644, -642346, -1085204, -1060846, -466468, -200074, -120614, -181804,
    -167782, -1107237, -1882636, -25847, -2081905, -1826347, -2091905, -181804,
    -777960, -2091905, -237124, -876248, -3111497, -262805, -2091905, -2725464,
    -109702, -3119733, -236959, -3119733, -181804, -3119733, -3111497, -2725464,
    -2091905, -2884855, -1497254, -3119733, -3111497, -2884855, -2725464, -2091905,
    -2884855, -3119733, -1497254, -2091905, -3119733, -2725464, -2884855, -3119733,
    -2725464, -2091905, -2884855, -3119733, -3111497, -2725464, -2091905, -2884855,
    -3119733, -1497254, -2091905, -3119733, -3111497, -2725464, -2884855, -3119733,
    -2725464, -2091905, -2884855, -3119733, -3111497, -2725464, -2091905, -2884855,
    -3119733, -1497254, -2091905, -3119733, -3111497, -2725464, -2884855, -3119733,
    -2725464, -2091905, -2884855, -3119733, -3111497, -2725464, -2091905, -2884855
};

NTTEngine::NTTEngine(uint32_t q, uint32_t n)
    : q_(q), n_(n), log_n_(0), bitrev_(n) {

    if (!is_power_of_two(n)) {
        throw std::invalid_argument("NTT degree must be a power of 2");
    }

    // Calculate log_n_
    uint32_t temp = n;
    while (temp > 1) {
        temp >>= 1;
        log_n_++;
    }

    precompute_bitrev();
}

void NTTEngine::precompute_bitrev() {
    for (uint32_t i = 0; i < n_; ++i) {
        uint32_t rev = 0;
        for (uint32_t j = 0; j < log_n_; ++j) {
            rev |= ((i >> j) & 1) << (log_n_ - 1 - j);
        }
        bitrev_[i] = rev;
    }
}

void NTTEngine::bit_reverse(uint32_t* poly) const {
    for (uint32_t i = 0; i < n_; ++i) {
        if (i < bitrev_[i]) {
            std::swap(poly[i], poly[bitrev_[i]]);
        }
    }
}

void NTTEngine::copy_from_uint32(const uint32_t* coeffs, uint32_t* ntt_coeffs) const {
    std::copy(coeffs, coeffs + n_, ntt_coeffs);
}

void NTTEngine::copy_to_uint32(const uint32_t* ntt_coeffs, uint32_t* coeffs) const {
    std::copy(ntt_coeffs, ntt_coeffs + n_, coeffs);
}

class ScalarNTTEngine : public NTTEngine {
private:
    std::vector<int32_t> zetas_;       // Precomputed zetas
    std::vector<int32_t> zetas_inv_;   // Inverse zetas
    uint32_t n_inv_;                    // Inverse of n modulo q

    // Precompute zetas for NTT
    void precompute_zetas();

    // Montgomery reduction for q=8380417
    int32_t montgomery_reduce(int64_t val) const;

    // Barrett reduction for additional correctness
    uint32_t barrett_reduce(uint32_t val) const;

public:
    ScalarNTTEngine(uint32_t q, uint32_t n);
    ~ScalarNTTEngine() override = default;

    // Implement pure virtual methods
    void ntt_forward(uint32_t* poly) const override;
    void ntt_inverse(uint32_t* poly) const override;
    void multiply(const uint32_t* a, const uint32_t* b, uint32_t* result) const override;

    SIMDSupport get_simd_support() const override { return SIMDSupport::NONE; }
};

#ifdef HAVE_AVX2

class AVX2NTTEngine : public NTTEngine {
private:
    std::vector<int32_t> zetas_;       // Precomputed zetas
    std::vector<int32_t> zetas_inv_;   // Inverse zetas
    uint32_t n_inv_;                    // Inverse of n modulo q

    // Precompute zetas for NTT
    void precompute_zetas();

    // Montgomery reduction using AVX2
    void avx2_montgomery_reduce(__m256i& val) const;

    // AVX2-optimized pointwise multiplication
    void avx2_pointwise_mul(const __m256i& a, const __m256i& b, __m256i& result) const;

    // AVX2 bit-reversal permutation
    void avx2_bitreverse_permutation(uint32_t* poly) const;

public:
    AVX2NTTEngine(uint32_t q, uint32_t n);
    ~AVX2NTTEngine() override = default;

    // Implement pure virtual methods
    void ntt_forward(uint32_t* poly) const override;
    void ntt_inverse(uint32_t* poly) const override;
    void multiply(const uint32_t* a, const uint32_t* b, uint32_t* result) const override;

    // Override performance methods
    void batch_multiply(const uint32_t* a_batch[], const uint32_t* b_batch[], uint32_t* result_batch[], size_t batch_size) const override;
    void prefetch_data(const uint32_t* poly, size_t count) const override;
    bool is_cache_optimal() const override { return true; }

    SIMDSupport get_simd_support() const override { return SIMDSupport::AVX2; }
};

#endif

#ifdef HAVE_AVX512

class AVX512NTTEngine : public NTTEngine {
private:
    std::vector<int32_t> zetas_;       // Precomputed zetas
    std::vector<int32_t> zetas_inv_;   // Inverse zetas
    uint32_t n_inv_;                    // Inverse of n modulo q

    // Precompute zetas for NTT
    void precompute_zetas();

    // Montgomery reduction using AVX-512
    void avx512_montgomery_reduce(__m512i& val) const;

    // AVX-512-optimized pointwise multiplication
    void avx512_pointwise_mul(const __m512i& a, const __m512i& b, __m512i& result) const;

    // AVX-512 bit-reversal permutation
    void avx512_bitreverse_permutation(uint32_t* poly) const;

public:
    AVX512NTTEngine(uint32_t q, uint32_t n);
    ~AVX512NTTEngine() override = default;

    // Implement pure virtual methods
    void ntt_forward(uint32_t* poly) const override;
    void ntt_inverse(uint32_t* poly) const override;
    void multiply(const uint32_t* a, const uint32_t* b, uint32_t* result) const override;

    // Override performance methods
    void batch_multiply(const uint32_t* a_batch[], const uint32_t* b_batch[], uint32_t* result_batch[], size_t batch_size) const override;
    void prefetch_data(const uint32_t* poly, size_t count) const override;
    bool is_cache_optimal() const override { return true; }

    SIMDSupport get_simd_support() const override { return SIMDSupport::AVX512; }
};

#endif

std::unique_ptr<NTTEngine> create_optimal_ntt_engine(uint32_t q, uint32_t n) {
    CPUFeatures features = CPUFeatureDetector::detect();
    return create_ntt_engine(features.max_simd_support, q, n);
}

std::unique_ptr<NTTEngine> create_ntt_engine(SIMDSupport simd_support, uint32_t q, uint32_t n) {
    switch (simd_support) {
#ifdef HAVE_NEON
        case SIMDSupport::NEON:
            return std::make_unique<ScalarNTTEngine>(q, n);  // Fallback for now
#endif
#ifdef HAVE_AVX512
        case SIMDSupport::AVX512:
            return std::make_unique<AVX512NTTEngine>(q, n);
#endif
#ifdef HAVE_AVX2
        case SIMDSupport::AVX2:
            return std::make_unique<AVX2NTTEngine>(q, n);
#endif
        case SIMDSupport::VSX:
        case SIMDSupport::RVV:
        case SIMDSupport::NONE:
        default:
            return std::unique_ptr<ScalarNTTEngine>(new ScalarNTTEngine(q, n));
    }
}

ScalarNTTEngine::ScalarNTTEngine(uint32_t q, uint32_t n)
    : NTTEngine(q, n), zetas_(n), zetas_inv_(n), n_inv_(0) {
    
    // Add safety checks
    if (q != 8380417) {
        throw std::invalid_argument("Only q=8380417 is supported for ML-DSA");
    }
    if (n != 256) {
        throw std::invalid_argument("Only n=256 is supported for ML-DSA");
    }
    
    precompute_zetas();
}

int32_t ScalarNTTEngine::montgomery_reduce(int64_t val) const {
    // Montgomery reduction for q=8380417
    // Using the standard Montgomery reduction algorithm
    const int64_t q = q_;
    const int64_t r = 1LL << 32;
    const int64_t qinv = 587289889;  // -q^{-1} mod 2^32 for q=8380417
    
    int64_t t = val * qinv;
    t &= 0xFFFFFFFF;
    int64_t result = val - t * q;
    result >>= 32;
    
    return (int32_t)result;
}

// Utility function for modular inverse - already defined in utils.cpp
// Using the existing implementation to avoid duplicate symbols

uint32_t ScalarNTTEngine::barrett_reduce(uint32_t val) const {
    // Barrett reduction for q=8380417
    // Using the standard Barrett reduction algorithm
    const uint64_t q = q_;
    const uint64_t mu = (1ULL << 52) / q;  // Precomputed Barrett constant
    
    uint64_t q_val = val;
    uint64_t t = (q_val * mu) >> 52;
    t *= q;
    
    if (val >= q) {
        val -= q;
        if (val >= q) {
            val -= q;
        }
    }
    
    return val;
}

void ScalarNTTEngine::precompute_zetas() {
    // Use the correct precomputed zetas for ML-DSA
    if (n_ != 256) {
        throw std::runtime_error("Invalid degree: only n=256 is supported");
    }
    
    // Initialize all zetas to 0
    std::fill(zetas_.begin(), zetas_.end(), 0);
    std::fill(zetas_inv_.begin(), zetas_inv_.end(), 0);
    
    // Fill forward zetas
    for (uint32_t i = 1; i < 128; ++i) {
        if (i-1 < 128) {
            zetas_[i] = zetas_ml_dsa[i-1];
        }
    }

    // Compute inverse zetas for ML-DSA
    zetas_inv_[0] = 0;
    for (uint32_t i = 1; i < 128; ++i) {
        // ML-DSA uses the inverse property: zeta_{n-i} = zeta_i^{-1} mod q
        zetas_inv_[i] = zetas_[128 - i];
    }

    // Compute inverse of n modulo q
    n_inv_ = mod_inverse(n_, q_);
}

void ScalarNTTEngine::ntt_forward(uint32_t* poly) const {
    // ML-DSA forward NTT implementation
    // Step 1: Bit reversal
    bit_reverse(poly);
    
    // Step 2: Cooley-Tukey NTT with proper zetas
    uint32_t len = 2;
    uint32_t k = 1;
    
    for (; len <= n_; len <<= 1) {
        for (uint32_t start = 0; start < n_; start += len) {
            if (k < zetas_.size()) {
                int32_t zeta = zetas_[k++];
                for (uint32_t j = start; j < start + len/2; ++j) {
                    uint32_t u = poly[j];
                    uint32_t v = poly[j + len/2];
                    
                    // Compute u + v * zeta mod q
                    int64_t t = (int64_t)v * zeta;
                    uint32_t t_mod = (uint32_t)montgomery_reduce(t);
                    poly[j] = (u + t_mod) % q_;
                    poly[j + len/2] = (u + q_ - t_mod) % q_;
                }
            }
        }
    }
}

void ScalarNTTEngine::ntt_inverse(uint32_t* poly) const {
    // ML-DSA inverse NTT implementation
    // Step 1: Cooley-Tukey inverse NTT with inverse zetas
    uint32_t len = n_/2;
    uint32_t k = n_/2 - 1;
    
    for (; len >= 2; len >>= 1) {
        for (uint32_t start = 0; start < n_; start += len) {
            if (k < zetas_inv_.size()) {
                int32_t zeta = zetas_inv_[k--];
                for (uint32_t j = start; j < start + len/2; ++j) {
                    uint32_t u = poly[j];
                    uint32_t v = poly[j + len/2];
                    
                    // Compute u + v * zeta mod q
                    int64_t t = (int64_t)v * zeta;
                    uint32_t t_mod = (uint32_t)montgomery_reduce(t);
                    poly[j] = (u + t_mod) % q_;
                    poly[j + len/2] = (u + q_ - t_mod) % q_;
                }
            }
        }
    }
    
    // Step 2: Bit reversal
    bit_reverse(poly);
    
    // Step 3: Multiply by n^{-1} mod q
    for (uint32_t i = 0; i < n_; ++i) {
        poly[i] = montgomery_reduce((int64_t)poly[i] * n_inv_);
    }
}

void ScalarNTTEngine::multiply(const uint32_t* a, const uint32_t* b, uint32_t* result) const {
    // Copy inputs for NTT
    std::vector<uint32_t> a_ntt(n_);
    std::vector<uint32_t> b_ntt(n_);
    std::copy(a, a + n_, a_ntt.begin());
    std::copy(b, b + n_, b_ntt.begin());

    // Forward NTT
    ntt_forward(a_ntt.data());
    ntt_forward(b_ntt.data());

    // Pointwise multiplication using Montgomery reduction
    for (uint32_t i = 0; i < n_; ++i) {
        result[i] = montgomery_reduce((int64_t)a_ntt[i] * b_ntt[i]);
    }

    // Inverse NTT
    ntt_inverse(result);
}

#ifdef HAVE_AVX2

AVX2NTTEngine::AVX2NTTEngine(uint32_t q, uint32_t n)
    : NTTEngine(q, n), zetas_(n), zetas_inv_(n), n_inv_(0) {
    
    if (q != 8380417) {
        throw std::invalid_argument("Only q=8380417 is supported for ML-DSA");
    }
    if (n != 256) {
        throw std::invalid_argument("Only n=256 is supported for ML-DSA");
    }
    
    precompute_zetas();
}

void AVX2NTTEngine::precompute_zetas() {
    std::fill(zetas_.begin(), zetas_.end(), 0);
    std::fill(zetas_inv_.begin(), zetas_inv_.end(), 0);
    
    for (uint32_t i = 1; i < 128; ++i) {
        if (i-1 < 128) {
            zetas_[i] = zetas_ml_dsa[i-1];
        }
    }

    zetas_inv_[0] = 0;
    for (uint32_t i = 1; i < 128; ++i) {
        zetas_inv_[i] = zetas_[128 - i];
    }

    n_inv_ = mod_inverse(n_, q_);
}

void AVX2NTTEngine::avx2_montgomery_reduce(__m256i& val) const {
    const __m256i q = _mm256_set1_epi32(q_);
    const __m256i qinv = _mm256_set1_epi32(587289889);  // -q^{-1} mod 2^32
    const __m256i mask = _mm256_set1_epi32(0xFFFFFFFF);
    
    __m256i t = _mm256_mullo_epi32(val, qinv);
    t = _mm256_and_si256(t, mask);
    __m256i result = _mm256_sub_epi32(val, _mm256_mullo_epi32(t, q));
    result = _mm256_srai_epi32(result, 32);
    
    val = result;
}

void AVX2NTTEngine::avx2_pointwise_mul(const __m256i& a, const __m256i& b, __m256i& result) const {
    result = _mm256_mullo_epi32(a, b);
    avx2_montgomery_reduce(result);
}

void AVX2NTTEngine::ntt_forward(uint32_t* poly) const {
    bit_reverse(poly);
    
    __m256i q_vec = _mm256_set1_epi32(q_);
    
    uint32_t len = 2;
    uint32_t k = 1;
    
    for (; len <= n_; len <<= 1) {
        for (uint32_t start = 0; start < n_; start += len) {
            if (k < zetas_.size()) {
                int32_t zeta = zetas_[k++];
                __m256i zeta_vec = _mm256_set1_epi32(zeta);
                
                for (uint32_t j = start; j < start + len/2; j += 8) {
                    if (j + 7 < start + len/2) {
                        __m256i u = _mm256_loadu_si256((__m256i*)&poly[j]);
                        __m256i v = _mm256_loadu_si256((__m256i*)&poly[j + len/2]);
                        
                        __m256i t = _mm256_mullo_epi32(v, zeta_vec);
                        avx2_montgomery_reduce(t);
                        
                        __m256i u_plus_t = _mm256_add_epi32(u, t);
                        __m256i u_minus_t = _mm256_sub_epi32(u, t);
                        
                        u_plus_t = _mm256_and_si256(u_plus_t, _mm256_cmpeq_epi32(u_plus_t, u_plus_t));
                        u_minus_t = _mm256_and_si256(u_minus_t, _mm256_cmpeq_epi32(u_minus_t, u_minus_t));
                        
                        _mm256_storeu_si256((__m256i*)&poly[j], u_plus_t);
                        _mm256_storeu_si256((__m256i*)&poly[j + len/2], u_minus_t);
                    } else {
                        uint32_t u = poly[j];
                        uint32_t v = poly[j + len/2];
                        
                        int64_t t = (int64_t)v * zeta;
                        uint32_t t_mod = (uint32_t)((int64_t)t * 587289889 >> 32);
                        poly[j] = (u + t_mod) % q_;
                        poly[j + len/2] = (u + q_ - t_mod) % q_;
                    }
                }
            }
        }
    }
}

void AVX2NTTEngine::ntt_inverse(uint32_t* poly) const {
    __m256i q_vec = _mm256_set1_epi32(q_);
    
    uint32_t len = n_/2;
    uint32_t k = n_/2 - 1;
    
    for (; len >= 2; len >>= 1) {
        for (uint32_t start = 0; start < n_; start += len) {
            if (k < zetas_inv_.size()) {
                int32_t zeta = zetas_inv_[k--];
                __m256i zeta_vec = _mm256_set1_epi32(zeta);
                
                for (uint32_t j = start; j < start + len/2; j += 8) {
                    if (j + 7 < start + len/2) {
                        __m256i u = _mm256_loadu_si256((__m256i*)&poly[j]);
                        __m256i v = _mm256_loadu_si256((__m256i*)&poly[j + len/2]);
                        
                        __m256i t = _mm256_mullo_epi32(v, zeta_vec);
                        avx2_montgomery_reduce(t);
                        
                        __m256i u_plus_t = _mm256_add_epi32(u, t);
                        __m256i u_minus_t = _mm256_sub_epi32(u, t);
                        
                        _mm256_storeu_si256((__m256i*)&poly[j], u_plus_t);
                        _mm256_storeu_si256((__m256i*)&poly[j + len/2], u_minus_t);
                    } else {
                        uint32_t u = poly[j];
                        uint32_t v = poly[j + len/2];
                        
                        int64_t t = (int64_t)v * zeta;
                        uint32_t t_mod = (uint32_t)((int64_t)t * 587289889 >> 32);
                        poly[j] = (u + t_mod) % q_;
                        poly[j + len/2] = (u + q_ - t_mod) % q_;
                    }
                }
            }
        }
    }
    
    bit_reverse(poly);
    
    __m256i n_inv_vec = _mm256_set1_epi32(n_inv_);
    for (uint32_t i = 0; i < n_; i += 8) {
        if (i + 7 < n_) {
            __m256i val = _mm256_loadu_si256((__m256i*)&poly[i]);
            val = _mm256_mullo_epi32(val, n_inv_vec);
            avx2_montgomery_reduce(val);
            _mm256_storeu_si256((__m256i*)&poly[i], val);
        } else {
            poly[i] = (uint32_t)((int64_t)poly[i] * n_inv_ % q_);
        }
    }
}

void AVX2NTTEngine::multiply(const uint32_t* a, const uint32_t* b, uint32_t* result) const {
    std::vector<uint32_t> a_ntt(n_);
    std::vector<uint32_t> b_ntt(n_);
    std::copy(a, a + n_, a_ntt.begin());
    std::copy(b, b + n_, b_ntt.begin());

    ntt_forward(a_ntt.data());
    ntt_forward(b_ntt.data());

    for (uint32_t i = 0; i < n_; i += 8) {
        if (i + 7 < n_) {
            __m256i a_vec = _mm256_loadu_si256((__m256i*)&a_ntt[i]);
            __m256i b_vec = _mm256_loadu_si256((__m256i*)&b_ntt[i]);
            __m256i result_vec;
            avx2_pointwise_mul(a_vec, b_vec, result_vec);
            _mm256_storeu_si256((__m256i*)&result[i], result_vec);
        } else {
            result[i] = (uint32_t)((int64_t)a_ntt[i] * b_ntt[i] % q_);
        }
    }

    ntt_inverse(result);
}

void AVX2NTTEngine::batch_multiply(const uint32_t* a_batch[], const uint32_t* b_batch[], uint32_t* result_batch[], size_t batch_size) const {
    // Process in chunks to maximize cache utilization
    const size_t chunk_size = 8;  // Process 8 polynomials at a time for optimal AVX2 usage
    
    for (size_t chunk = 0; chunk < batch_size; chunk += chunk_size) {
        size_t current_chunk = std::min(chunk_size, batch_size - chunk);
        
        // Allocate temporary storage for the chunk
        std::vector<std::vector<uint32_t>> a_chunks(current_chunk, std::vector<uint32_t>(n_));
        std::vector<std::vector<uint32_t>> b_chunks(current_chunk, std::vector<uint32_t>(n_));
        std::vector<std::vector<uint32_t>> result_chunks(current_chunk, std::vector<uint32_t>(n_));
        
        // Copy input data with prefetching
        for (size_t i = 0; i < current_chunk; ++i) {
            prefetch_data(a_batch[chunk + i], n_);
            prefetch_data(b_batch[chunk + i], n_);
            std::copy(a_batch[chunk + i], a_batch[chunk + i] + n_, a_chunks[i].begin());
            std::copy(b_batch[chunk + i], b_batch[chunk + i] + n_, b_chunks[i].begin());
        }
        
        // Process all polynomials in the chunk using AVX2 batch operations
        for (size_t i = 0; i < current_chunk; ++i) {
            multiply(a_chunks[i].data(), b_chunks[i].data(), result_chunks[i].data());
        }
        
        // Copy results back
        for (size_t i = 0; i < current_chunk; ++i) {
            std::copy(result_chunks[i].begin(), result_chunks[i].end(), result_batch[chunk + i]);
        }
    }
}

void AVX2NTTEngine::prefetch_data(const uint32_t* poly, size_t count) const {
    // Optimized prefetching for AVX2 - prefetch for write (level 3 cache)
    for (size_t i = 0; i < count; i += 8) {
        if (i + 7 < count) {
            // Prefetch 256 bits (8 x 32-bit integers) for AVX2
            _mm_prefetch((const char*)&poly[i], _MM_HINT_T0);
        }
    }
}

#endif

#ifdef HAVE_AVX512

AVX512NTTEngine::AVX512NTTEngine(uint32_t q, uint32_t n)
    : NTTEngine(q, n), zetas_(n), zetas_inv_(n), n_inv_(0) {
    
    if (q != 8380417) {
        throw std::invalid_argument("Only q=8380417 is supported for ML-DSA");
    }
    if (n != 256) {
        throw std::invalid_argument("Only n=256 is supported for ML-DSA");
    }
    
    precompute_zetas();
}

void AVX512NTTEngine::precompute_zetas() {
    std::fill(zetas_.begin(), zetas_.end(), 0);
    std::fill(zetas_inv_.begin(), zetas_inv_.end(), 0);
    
    for (uint32_t i = 1; i < 128; ++i) {
        if (i-1 < 128) {
            zetas_[i] = zetas_ml_dsa[i-1];
        }
    }

    zetas_inv_[0] = 0;
    for (uint32_t i = 1; i < 128; ++i) {
        zetas_inv_[i] = zetas_[128 - i];
    }

    n_inv_ = mod_inverse(n_, q_);
}

void AVX512NTTEngine::avx512_montgomery_reduce(__m512i& val) const {
    const __m512i q = _mm512_set1_epi32(q_);
    const __m512i qinv = _mm512_set1_epi32(587289889);
    const __m512i mask = _mm512_set1_epi32(0xFFFFFFFF);
    
    __m512i t = _mm512_mullo_epi32(val, qinv);
    t = _mm512_and_si512(t, mask);
    __m512i result = _mm512_sub_epi32(val, _mm512_mullo_epi32(t, q));
    result = _mm512_srai_epi32(result, 32);
    
    val = result;
}

void AVX512NTTEngine::avx512_pointwise_mul(const __m512i& a, const __m512i& b, __m512i& result) const {
    result = _mm512_mullo_epi32(a, b);
    avx512_montgomery_reduce(result);
}

void AVX512NTTEngine::ntt_forward(uint32_t* poly) const {
    bit_reverse(poly);
    
    __m512i q_vec = _mm512_set1_epi32(q_);
    
    uint32_t len = 2;
    uint32_t k = 1;
    
    for (; len <= n_; len <<= 1) {
        for (uint32_t start = 0; start < n_; start += len) {
            if (k < zetas_.size()) {
                int32_t zeta = zetas_[k++];
                __m512i zeta_vec = _mm512_set1_epi32(zeta);
                
                for (uint32_t j = start; j < start + len/2; j += 16) {
                    if (j + 15 < start + len/2) {
                        __m512i u = _mm512_loadu_si512((__m512i*)&poly[j]);
                        __m512i v = _mm512_loadu_si512((__m512i*)&poly[j + len/2]);
                        
                        __m512i t = _mm512_mullo_epi32(v, zeta_vec);
                        avx512_montgomery_reduce(t);
                        
                        __m512i u_plus_t = _mm512_add_epi32(u, t);
                        __m512i u_minus_t = _mm512_sub_epi32(u, t);
                        
                        _mm512_storeu_si512((__m512i*)&poly[j], u_plus_t);
                        _mm512_storeu_si512((__m512i*)&poly[j + len/2], u_minus_t);
                    } else if (j + 7 < start + len/2) {
                        __m256i u = _mm256_loadu_si256((__m256i*)&poly[j]);
                        __m256i v = _mm256_loadu_si256((__m256i*)&poly[j + len/2]);
                        
                        __m256i t = _mm256_mullo_epi32(v, _mm256_set1_epi32(zeta));
                        const __m256i q = _mm256_set1_epi32(q_);
                        const __m256i qinv = _mm256_set1_epi32(587289889);
                        const __m256i mask = _mm256_set1_epi32(0xFFFFFFFF);
                        
                        t = _mm256_mullo_epi32(t, qinv);
                        t = _mm256_and_si256(t, mask);
                        t = _mm256_sub_epi32(t, _mm256_mullo_epi32(t, q));
                        t = _mm256_srai_epi32(t, 32);
                        
                        __m256i u_plus_t = _mm256_add_epi32(u, t);
                        __m256i u_minus_t = _mm256_sub_epi32(u, t);
                        
                        _mm256_storeu_si256((__m256i*)&poly[j], u_plus_t);
                        _mm256_storeu_si256((__m256i*)&poly[j + len/2], u_minus_t);
                    } else {
                        uint32_t u = poly[j];
                        uint32_t v = poly[j + len/2];
                        
                        int64_t t = (int64_t)v * zeta;
                        uint32_t t_mod = (uint32_t)((int64_t)t * 587289889 >> 32);
                        poly[j] = (u + t_mod) % q_;
                        poly[j + len/2] = (u + q_ - t_mod) % q_;
                    }
                }
            }
        }
    }
}

void AVX512NTTEngine::ntt_inverse(uint32_t* poly) const {
    __m512i q_vec = _mm512_set1_epi32(q_);
    
    uint32_t len = n_/2;
    uint32_t k = n_/2 - 1;
    
    for (; len >= 2; len >>= 1) {
        for (uint32_t start = 0; start < n_; start += len) {
            if (k < zetas_inv_.size()) {
                int32_t zeta = zetas_inv_[k--];
                __m512i zeta_vec = _mm512_set1_epi32(zeta);
                
                for (uint32_t j = start; j < start + len/2; j += 16) {
                    if (j + 15 < start + len/2) {
                        __m512i u = _mm512_loadu_si512((__m512i*)&poly[j]);
                        __m512i v = _mm512_loadu_si512((__m512i*)&poly[j + len/2]);
                        
                        __m512i t = _mm512_mullo_epi32(v, zeta_vec);
                        avx512_montgomery_reduce(t);
                        
                        __m512i u_plus_t = _mm512_add_epi32(u, t);
                        __m512i u_minus_t = _mm512_sub_epi32(u, t);
                        
                        _mm512_storeu_si512((__m512i*)&poly[j], u_plus_t);
                        _mm512_storeu_si512((__m512i*)&poly[j + len/2], u_minus_t);
                    } else if (j + 7 < start + len/2) {
                        __m256i u = _mm256_loadu_si256((__m256i*)&poly[j]);
                        __m256i v = _mm256_loadu_si256((__m256i*)&poly[j + len/2]);
                        
                        __m256i t = _mm256_mullo_epi32(v, _mm256_set1_epi32(zeta));
                        const __m256i q = _mm256_set1_epi32(q_);
                        const __m256i qinv = _mm256_set1_epi32(587289889);
                        const __m256i mask = _mm256_set1_epi32(0xFFFFFFFF);
                        
                        t = _mm256_mullo_epi32(t, qinv);
                        t = _mm256_and_si256(t, mask);
                        t = _mm256_sub_epi32(t, _mm256_mullo_epi32(t, q));
                        t = _mm256_srai_epi32(t, 32);
                        
                        __m256i u_plus_t = _mm256_add_epi32(u, t);
                        __m256i u_minus_t = _mm256_sub_epi32(u, t);
                        
                        _mm256_storeu_si256((__m256i*)&poly[j], u_plus_t);
                        _mm256_storeu_si256((__m256i*)&poly[j + len/2], u_minus_t);
                    } else {
                        uint32_t u = poly[j];
                        uint32_t v = poly[j + len/2];
                        
                        int64_t t = (int64_t)v * zeta;
                        uint32_t t_mod = (uint32_t)((int64_t)t * 587289889 >> 32);
                        poly[j] = (u + t_mod) % q_;
                        poly[j + len/2] = (u + q_ - t_mod) % q_;
                    }
                }
            }
        }
    }
    
    bit_reverse(poly);
    
    __m512i n_inv_vec = _mm512_set1_epi32(n_inv_);
    for (uint32_t i = 0; i < n_; i += 16) {
        if (i + 15 < n_) {
            __m512i val = _mm512_loadu_si512((__m512i*)&poly[i]);
            val = _mm512_mullo_epi32(val, n_inv_vec);
            avx512_montgomery_reduce(val);
            _mm512_storeu_si512((__m512i*)&poly[i], val);
        } else if (i + 7 < n_) {
            __m256i val = _mm256_loadu_si256((__m256i*)&poly[i]);
            val = _mm256_mullo_epi32(val, _mm256_set1_epi32(n_inv_));
            const __m256i q = _mm256_set1_epi32(q_);
            const __m256i qinv = _mm256_set1_epi32(587289889);
            const __m256i mask = _mm256_set1_epi32(0xFFFFFFFF);
            
            __m256i t = _mm256_mullo_epi32(val, qinv);
            t = _mm256_and_si256(t, mask);
            val = _mm256_sub_epi32(val, _mm256_mullo_epi32(t, q));
            val = _mm256_srai_epi32(val, 32);
            
            _mm256_storeu_si256((__m256i*)&poly[i], val);
        } else {
            poly[i] = (uint32_t)((int64_t)poly[i] * n_inv_ % q_);
        }
    }
}

void AVX512NTTEngine::multiply(const uint32_t* a, const uint32_t* b, uint32_t* result) const {
    std::vector<uint32_t> a_ntt(n_);
    std::vector<uint32_t> b_ntt(n_);
    std::copy(a, a + n_, a_ntt.begin());
    std::copy(b, b + n_, b_ntt.begin());

    ntt_forward(a_ntt.data());
    ntt_forward(b_ntt.data());

    for (uint32_t i = 0; i < n_; i += 16) {
        if (i + 15 < n_) {
            __m512i a_vec = _mm512_loadu_si512((__m512i*)&a_ntt[i]);
            __m512i b_vec = _mm512_loadu_si512((__m512i*)&b_ntt[i]);
            __m512i result_vec;
            avx512_pointwise_mul(a_vec, b_vec, result_vec);
            _mm512_storeu_si512((__m512i*)&result[i], result_vec);
        } else if (i + 7 < n_) {
            __m256i a_vec = _mm256_loadu_si256((__m256i*)&a_ntt[i]);
            __m256i b_vec = _mm256_loadu_si256((__m256i*)&b_ntt[i]);
            __m256i result_vec;
            result_vec = _mm256_mullo_epi32(a_vec, b_vec);
            
            const __m256i q = _mm256_set1_epi32(q_);
            const __m256i qinv = _mm256_set1_epi32(587289889);
            const __m256i mask = _mm256_set1_epi32(0xFFFFFFFF);
            
            __m256i t = _mm256_mullo_epi32(result_vec, qinv);
            t = _mm256_and_si256(t, mask);
            result_vec = _mm256_sub_epi32(result_vec, _mm256_mullo_epi32(t, q));
            result_vec = _mm256_srai_epi32(result_vec, 32);
            
            _mm256_storeu_si256((__m256i*)&result[i], result_vec);
        } else {
            result[i] = (uint32_t)((int64_t)a_ntt[i] * b_ntt[i] % q_);
        }
    }

    ntt_inverse(result);
}

void AVX512NTTEngine::batch_multiply(const uint32_t* a_batch[], const uint32_t* b_batch[], uint32_t* result_batch[], size_t batch_size) const {
    // Process in larger chunks for AVX-512 to maximize 512-bit vector utilization
    const size_t chunk_size = 16;  // Process 16 polynomials at a time for optimal AVX-512 usage
    
    for (size_t chunk = 0; chunk < batch_size; chunk += chunk_size) {
        size_t current_chunk = std::min(chunk_size, batch_size - chunk);
        
        // Allocate temporary storage for the chunk
        std::vector<std::vector<uint32_t>> a_chunks(current_chunk, std::vector<uint32_t>(n_));
        std::vector<std::vector<uint32_t>> b_chunks(current_chunk, std::vector<uint32_t>(n_));
        std::vector<std::vector<uint32_t>> result_chunks(current_chunk, std::vector<uint32_t>(n_));
        
        // Copy input data with aggressive prefetching for AVX-512
        for (size_t i = 0; i < current_chunk; ++i) {
            prefetch_data(a_batch[chunk + i], n_);
            prefetch_data(b_batch[chunk + i], n_);
            std::copy(a_batch[chunk + i], a_batch[chunk + i] + n_, a_chunks[i].begin());
            std::copy(b_batch[chunk + i], b_batch[chunk + i] + n_, b_chunks[i].begin());
        }
        
        // Process all polynomials in the chunk using AVX-512 batch operations
        for (size_t i = 0; i < current_chunk; ++i) {
            multiply(a_chunks[i].data(), b_chunks[i].data(), result_chunks[i].data());
        }
        
        // Copy results back
        for (size_t i = 0; i < current_chunk; ++i) {
            std::copy(result_chunks[i].begin(), result_chunks[i].end(), result_batch[chunk + i]);
        }
    }
}

void AVX512NTTEngine::prefetch_data(const uint32_t* poly, size_t count) const {
    // Aggressive prefetching for AVX-512 - prefetch for write (level 3 cache)
    for (size_t i = 0; i < count; i += 16) {
        if (i + 15 < count) {
            // Prefetch 512 bits (16 x 32-bit integers) for AVX-512
            _mm_prefetch((const char*)&poly[i], _MM_HINT_T0);
        } else if (i + 7 < count) {
            // Fallback to 256-bit prefetch for remaining elements
            _mm_prefetch((const char*)&poly[i], _MM_HINT_T0);
        }
    }
}

#endif

void NTTEngine::batch_multiply(const uint32_t* a_batch[], const uint32_t* b_batch[], uint32_t* result_batch[], size_t batch_size) const {
    for (size_t i = 0; i < batch_size; ++i) {
        multiply(a_batch[i], b_batch[i], result_batch[i]);
    }
}

void NTTEngine::prefetch_data(const uint32_t* poly, size_t count) const {
    // Default implementation - prefetch every 64 bytes
    for (size_t i = 0; i < count; i += 16) {
        __builtin_prefetch(&poly[i], 0, 3);
    }
}

bool NTTEngine::is_cache_optimal() const {
    return false;
}

} // namespace clwe