#include "clwe/ntt_scalar.hpp"
#include "clwe/utils.hpp"
#include <algorithm>
#include <iostream>
#include <cstring>
#include <unordered_map>

namespace clwe {

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

ScalarNTTEngine::ScalarNTTEngine(uint32_t q, uint32_t n)
    : NTTEngine(q, n), zetas_(n), zetas_inv_(n), n_inv_(0) {

    if (q != 8380417) {
        throw std::invalid_argument("Only q=8380417 is supported for ML-DSA");
    }
    if (n != 256) {
        throw std::invalid_argument("Only n=256 is supported for ML-DSA");
    }

    precompute_zetas();
}

int32_t ScalarNTTEngine::montgomery_reduce(int64_t val) const {
    const int64_t q = q_;
    const int64_t r = 1LL << 32;
    const int64_t qinv = 587289889;

    int64_t t = val * qinv;
    t &= 0xFFFFFFFF;
    int64_t result = val - t * q;
    result >>= 32;

    return (int32_t)result;
}

uint32_t ScalarNTTEngine::barrett_reduce(uint32_t val) const {
    const uint64_t q = q_;
    const uint64_t mu = (1ULL << 52) / q;

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
    if (n_ != 256) {
        throw std::runtime_error("Invalid degree: only n=256 is supported");
    }

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

void ScalarNTTEngine::ntt_forward(uint32_t* poly) const {
    bit_reverse(poly);

    uint32_t len = 2;
    uint32_t k = 1;

    for (; len <= n_; len <<= 1) {
        for (uint32_t start = 0; start < n_; start += len) {
            if (k < zetas_.size()) {
                int32_t zeta = zetas_[k++];
                for (uint32_t j = start; j < start + len/2; ++j) {
                    uint32_t u = poly[j];
                    uint32_t v = poly[j + len/2];

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
    uint32_t len = n_/2;
    uint32_t k = n_/2 - 1;

    for (; len >= 2; len >>= 1) {
        for (uint32_t start = 0; start < n_; start += len) {
            if (k < zetas_inv_.size()) {
                int32_t zeta = zetas_inv_[k--];
                for (uint32_t j = start; j < start + len/2; ++j) {
                    uint32_t u = poly[j];
                    uint32_t v = poly[j + len/2];

                    int64_t t = (int64_t)v * zeta;
                    uint32_t t_mod = (uint32_t)montgomery_reduce(t);
                    poly[j] = (u + t_mod) % q_;
                    poly[j + len/2] = (u + q_ - t_mod) % q_;
                }
            }
        }
    }

    bit_reverse(poly);

    for (uint32_t i = 0; i < n_; ++i) {
        poly[i] = montgomery_reduce((int64_t)poly[i] * n_inv_);
    }
}

void ScalarNTTEngine::multiply(const uint32_t* a, const uint32_t* b, uint32_t* result) const {
    std::vector<uint32_t> a_ntt(n_);
    std::vector<uint32_t> b_ntt(n_);
    std::copy(a, a + n_, a_ntt.begin());
    std::copy(b, b + n_, b_ntt.begin());

    ntt_forward(a_ntt.data());
    ntt_forward(b_ntt.data());

    for (uint32_t i = 0; i < n_; ++i) {
        result[i] = montgomery_reduce((int64_t)a_ntt[i] * b_ntt[i]);
    }

    ntt_inverse(result);
}


} // namespace clwe