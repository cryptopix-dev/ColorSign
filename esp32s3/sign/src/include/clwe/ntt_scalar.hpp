#ifndef NTT_SCALAR_HPP
#define NTT_SCALAR_HPP

#include "ntt_engine.hpp"
#include <cstdint>
#include <vector>

#ifdef ESP_PLATFORM
#include <esp_attr.h>
#endif

namespace clwe {

class ScalarNTTEngine : public NTTEngine {
private:
    std::vector<int32_t> zetas_;
    std::vector<int32_t> zetas_inv_;
    uint32_t n_inv_;

    void precompute_zetas();

    int32_t montgomery_reduce(int64_t val) const;

    uint32_t barrett_reduce(uint32_t val) const;

public:
    ScalarNTTEngine(uint32_t q, uint32_t n);
    ~ScalarNTTEngine() override = default;

    void ntt_forward(uint32_t* poly) const override;
    void ntt_inverse(uint32_t* poly) const override;
    void multiply(const uint32_t* a, const uint32_t* b, uint32_t* result) const override;

    SIMDSupport get_simd_support() const override { return SIMDSupport::NONE; }
};

} // namespace clwe

#endif // NTT_SCALAR_HPP