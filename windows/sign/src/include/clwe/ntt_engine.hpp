#ifndef NTT_ENGINE_HPP
#define NTT_ENGINE_HPP

#include <cstdint>
#include <vector>
#include <memory>

namespace clwe {

// SIMD support enumeration
enum class SIMDSupport {
    NONE,     // No SIMD support
    AVX2,     // AVX2 (256-bit vectors)
    AVX512,   // AVX-512 (512-bit vectors)
    NEON,     // ARM NEON
    RVV,      // RISC-V Vector extension
    VSX       // PowerPC VSX
};

// Forward declarations
class Polynomial;

// Abstract base class for NTT engines
class NTTEngine {
public:
    NTTEngine(uint32_t q, uint32_t n);
    virtual ~NTTEngine() = default;

    // Disable copy and assignment
    NTTEngine(const NTTEngine&) = delete;
    NTTEngine& operator=(const NTTEngine&) = delete;

    // Pure virtual methods that must be implemented by derived classes
    virtual void ntt_forward(uint32_t* poly) const = 0;
    virtual void ntt_inverse(uint32_t* poly) const = 0;
    virtual void multiply(const uint32_t* a, const uint32_t* b, uint32_t* result) const = 0;

    // Virtual methods with default implementations
    virtual bool has_avx512() const { return false; }
    virtual SIMDSupport get_simd_support() const = 0;

    // Performance optimization methods
    virtual void batch_multiply(const uint32_t* a_batch[], const uint32_t* b_batch[], uint32_t* result_batch[], size_t batch_size) const;
    virtual void prefetch_data(const uint32_t* poly, size_t count) const;
    virtual bool is_cache_optimal() const;

    // Utility functions
    virtual void bit_reverse(uint32_t* poly) const;
    virtual void copy_from_uint32(const uint32_t* coeffs, uint32_t* ntt_coeffs) const;
    virtual void copy_to_uint32(const uint32_t* ntt_coeffs, uint32_t* coeffs) const;

    // Getters
    uint32_t modulus() const { return q_; }
    uint32_t degree() const { return n_; }
    uint32_t log_degree() const { return log_n_; }

protected:
    uint32_t q_;           // Modulus
    uint32_t n_;           // Degree (power of 2)
    uint32_t log_n_;       // log2(n_)
    std::vector<uint32_t> bitrev_;  // Bit reversal table

    // Precompute bit reversal table
    void precompute_bitrev();
};

// Factory function to create optimal NTT engine
std::unique_ptr<NTTEngine> create_optimal_ntt_engine(uint32_t q, uint32_t n);

// Factory function to create specific NTT engine
std::unique_ptr<NTTEngine> create_ntt_engine(SIMDSupport simd_support, uint32_t q, uint32_t n);

} // namespace clwe

#endif // NTT_ENGINE_HPP