#include "../include/clwe/utils.hpp"
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <vector>
#include <array>

#ifdef __APPLE__
#include <Security/SecRandom.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#endif

#ifdef __linux__
#include <sys/random.h>
#endif

namespace clwe {

// Keccak constants
static const uint64_t KECCAK_RC[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808AULL,
    0x8000000080008000ULL, 0x000000000000808BULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008AULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000AULL,
    0x000000008000808BULL, 0x800000000000008BULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800AULL, 0x800000008000000AULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

static const int KECCAK_RHO_OFFSETS[25] = {
    0, 1, 62, 28, 27,
    36, 44, 6, 55, 20,
    3, 10, 43, 25, 39,
    41, 45, 15, 21, 8,
    18, 2, 61, 56, 14
};

// Keccak-f[1600] permutation
static void keccak_f1600(uint64_t state[25]) {
    uint64_t C[5], D[5], B[25];

    for (int round = 0; round < 24; ++round) {
        // θ (theta) step
        for (int x = 0; x < 5; ++x) {
            C[x] = state[x] ^ state[x + 5] ^ state[x + 10] ^ state[x + 15] ^ state[x + 20];
        }
        for (int x = 0; x < 5; ++x) {
            D[x] = C[(x + 4) % 5] ^ ((C[(x + 1) % 5] << 1) | (C[(x + 1) % 5] >> 63));
        }
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 5; ++y) {
                state[x + 5 * y] ^= D[x];
            }
        }

        // ρ (rho) step - rotations
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 5; ++y) {
                int index = x + 5 * y;
                int offset = KECCAK_RHO_OFFSETS[index];
                B[y + 5 * ((2 * x + 3 * y) % 5)] = (state[index] << offset) | (state[index] >> (64 - offset));
            }
        }

        // π (pi) step - permutation (already done in the B assignment above)

        // χ (chi) step
        for (int y = 0; y < 5; ++y) {
            uint64_t T[5];
            for (int x = 0; x < 5; ++x) {
                T[x] = B[x + 5 * y];
            }
            for (int x = 0; x < 5; ++x) {
                state[x + 5 * y] = T[x] ^ ((~T[(x + 1) % 5]) & T[(x + 2) % 5]);
            }
        }

        // ι (iota) step
        state[0] ^= KECCAK_RC[round];
    }
}

// Sponge construction helper functions
static void absorb_bytes(uint64_t state[25], size_t rate_bytes, const uint8_t* data, size_t len, size_t& offset) {
    while (len > 0) {
        size_t chunk = std::min(len, rate_bytes - offset);
        for (size_t i = 0; i < chunk; ++i) {
            size_t state_byte_index = offset + i;
            size_t word_index = state_byte_index / 8;
            size_t byte_in_word = state_byte_index % 8;
            state[word_index] ^= static_cast<uint64_t>(data[i]) << (byte_in_word * 8);
        }
        offset += chunk;
        data += chunk;
        len -= chunk;

        if (offset == rate_bytes) {
            keccak_f1600(state);
            offset = 0;
        }
    }
}

static void squeeze_bytes(uint64_t state[25], size_t rate_bytes, uint8_t* out, size_t len, size_t& offset) {
    while (len > 0) {
        size_t chunk = std::min(len, rate_bytes - offset);
        for (size_t i = 0; i < chunk; ++i) {
            size_t state_byte_index = offset + i;
            size_t word_index = state_byte_index / 8;
            size_t byte_in_word = state_byte_index % 8;
            out[i] = (state[word_index] >> (byte_in_word * 8)) & 0xFF;
        }
        offset += chunk;
        out += chunk;
        len -= chunk;

        if (offset == rate_bytes) {
            keccak_f1600(state);
            offset = 0;
        }
    }
}

// Secure random bytes using platform-specific APIs
void secure_random_bytes(uint8_t* buffer, size_t len) {
#ifdef __APPLE__
    if (SecRandomCopyBytes(kSecRandomDefault, len, buffer) != 0) {
        throw std::runtime_error("Failed to generate secure random bytes on macOS");
    }
#elif defined(_WIN32)
    if (BCryptGenRandom(NULL, buffer, len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
        throw std::runtime_error("Failed to generate secure random bytes on Windows");
    }
#elif defined(__linux__)
    if (getrandom(buffer, len, 0) != static_cast<ssize_t>(len)) {
        throw std::runtime_error("Failed to generate secure random bytes on Linux");
    }
#else
    throw std::runtime_error("Secure random not implemented for this platform");
#endif
}

// SHAKE256 hash function
std::vector<uint8_t> shake256(const std::vector<uint8_t>& input, size_t output_len) {
    uint64_t state[25] = {0};
    size_t rate_bytes = 136;  // 1088 bits / 8 = 136 bytes for SHAKE256
    size_t offset = 0;

    // Absorb input
    if (!input.empty()) {
        absorb_bytes(state, rate_bytes, input.data(), input.size(), offset);
    }

    // Apply SHAKE padding: XOR domain separation byte 0x1F into current position
    uint8_t* state_bytes = reinterpret_cast<uint8_t*>(state);
    state_bytes[offset] ^= 0x1F;

    // Pad with zeros to fill the rate
    for (size_t i = offset + 1; i < rate_bytes; ++i) {
        state_bytes[i] = 0;
    }

    // Apply domain separation bit
    state_bytes[rate_bytes - 1] ^= 0x80;

    // Apply permutation after padding
    keccak_f1600(state);

    // Squeeze output
    std::vector<uint8_t> output(output_len);
    offset = 0;
    squeeze_bytes(state, rate_bytes, output.data(), output_len, offset);

    return output;
}

// SHAKE128Sampler implementation
SHAKE128Sampler::SHAKE128Sampler() : rate_bytes_(168), offset_(0) {  // 1344 bits / 8 = 168 bytes for SHAKE128
    reset();
}

SHAKE128Sampler::~SHAKE128Sampler() {
    // No dynamic memory to clean up
}

void SHAKE128Sampler::reset() {
    std::memset(state_, 0, sizeof(state_));
    offset_ = 0;
}

void SHAKE128Sampler::keccak_f1600() {
    ::clwe::keccak_f1600(state_);
}

void SHAKE128Sampler::absorb(const uint8_t* data, size_t len) {
    ::clwe::absorb_bytes(state_, rate_bytes_, data, len, offset_);
}

void SHAKE128Sampler::pad_and_absorb() {
    // XOR domain separation byte 0x1F into current position
    uint8_t* state_bytes = reinterpret_cast<uint8_t*>(state_);
    state_bytes[offset_] ^= 0x1F;

    // Pad with zeros to fill the rate
    for (size_t i = offset_ + 1; i < rate_bytes_; ++i) {
        state_bytes[i] = 0;
    }

    // Apply domain separation bit
    state_bytes[rate_bytes_ - 1] ^= 0x80;

    // Apply permutation
    keccak_f1600();

    offset_ = 0;  // Reset for squeezing
}

void SHAKE128Sampler::init(const uint8_t* seed, size_t seed_len) {
    reset();
    if (seed_len > 0) {
        absorb(seed, seed_len);
    }
    pad_and_absorb();
    offset_ = 0;  // Reset for squeezing
}

void SHAKE128Sampler::squeeze(uint8_t* out, size_t len) {
    ::clwe::squeeze_bytes(state_, rate_bytes_, out, len, offset_);
}

uint32_t SHAKE128Sampler::sample_uniform(uint32_t modulus) {
    // Sample uniformly from [0, modulus)
    uint32_t result = 0;
    uint32_t bits_needed = 0;
    uint32_t temp = modulus - 1;
    while (temp > 0) {
        bits_needed++;
        temp >>= 1;
    }

    while (true) {
        uint8_t bytes[4];
        squeeze(bytes, 4);
        result = (bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24));
        result &= (1U << bits_needed) - 1;
        if (result < modulus) {
            return result;
        }
    }
}

// SHAKE256Sampler implementation
SHAKE256Sampler::SHAKE256Sampler() : rate_bytes_(136), offset_(0) {  // 1088 bits / 8 = 136 bytes for SHAKE256
    reset();
}

SHAKE256Sampler::~SHAKE256Sampler() {
    // No dynamic memory to clean up
}

void SHAKE256Sampler::reset() {
    std::memset(state_, 0, sizeof(state_));
    offset_ = 0;
}

void SHAKE256Sampler::keccak_f1600() {
    ::clwe::keccak_f1600(state_);
}

void SHAKE256Sampler::absorb(const uint8_t* data, size_t len) {
    ::clwe::absorb_bytes(state_, rate_bytes_, data, len, offset_);
}

void SHAKE256Sampler::pad_and_absorb() {
    // XOR domain separation byte 0x1F into current position
    uint8_t* state_bytes = reinterpret_cast<uint8_t*>(state_);
    state_bytes[offset_] ^= 0x1F;

    // Pad with zeros to fill the rate
    for (size_t i = offset_ + 1; i < rate_bytes_; ++i) {
        state_bytes[i] = 0;
    }

    // Apply domain separation bit
    state_bytes[rate_bytes_ - 1] ^= 0x80;

    // Apply permutation
    keccak_f1600();

    offset_ = 0;  // Reset for squeezing
}

void SHAKE256Sampler::init(const uint8_t* seed, size_t seed_len) {
    reset();
    if (seed_len > 0) {
        absorb(seed, seed_len);
    }
    pad_and_absorb();
    offset_ = 0;  // Reset for squeezing
}

void SHAKE256Sampler::squeeze(uint8_t* out, size_t len) {
    ::clwe::squeeze_bytes(state_, rate_bytes_, out, len, offset_);
}

int32_t SHAKE256Sampler::sample_binomial_coefficient(uint32_t eta) {
    // Sample from centered binomial distribution B(2η, 0.5) - η
    int32_t sum = 0;
    for (uint32_t i = 0; i < eta; ++i) {
        uint8_t byte;
        squeeze(&byte, 1);
        sum += (byte & 1);
    }
    for (uint32_t i = 0; i < eta; ++i) {
        uint8_t byte;
        squeeze(&byte, 1);
        sum -= (byte & 1);
    }
    return sum;
}

void SHAKE256Sampler::sample_polynomial_binomial(uint32_t* coeffs, size_t degree,
                                                uint32_t eta, uint32_t modulus) {
    for (size_t i = 0; i < degree; ++i) {
        int32_t coeff = sample_binomial_coefficient(eta);
        coeffs[i] = (coeff % modulus + modulus) % modulus;  // Ensure non-negative
    }
}

uint32_t SHAKE256Sampler::sample_uniform(uint32_t modulus) {
    // Sample uniformly from [0, modulus)
    uint32_t result = 0;
    uint32_t bits_needed = 0;
    uint32_t temp = modulus - 1;
    while (temp > 0) {
        bits_needed++;
        temp >>= 1;
    }

    while (true) {
        uint8_t bytes[4];
        squeeze(bytes, 4);
        result = (bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24));
        result &= (1U << bits_needed) - 1;
        if (result < modulus) {
            return result;
        }
    }
}

void SHAKE256Sampler::sample_polynomial_uniform(uint32_t* coeffs, size_t degree, uint32_t modulus) {
    for (size_t i = 0; i < degree; ++i) {
        coeffs[i] = sample_uniform(modulus);
    }
}

void SHAKE256Sampler::random_bytes(uint8_t* out, size_t len) {
    squeeze(out, len);
}

// Modular arithmetic utilities
uint32_t mod_inverse(uint32_t a, uint32_t m) {
    int64_t m0 = m, t, q;
    int64_t x0 = 0, x1 = 1;
    if (m == 1) return 0;
    while (a > 1) {
        q = a / m;
        t = m;
        m = a % m, a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    if (x1 < 0) x1 += m0;
    return static_cast<uint32_t>(x1);
}

uint32_t mod_pow(uint32_t base, uint32_t exp, uint32_t mod) {
    uint32_t result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) {
            result = (static_cast<uint64_t>(result) * base) % mod;
        }
        base = (static_cast<uint64_t>(base) * base) % mod;
        exp >>= 1;
    }
    return result;
}

// ML-DSA specific utilities
void compute_high_bits(const std::vector<uint32_t>& w, std::vector<uint32_t>& w1, uint32_t d, uint32_t q) {
    uint32_t shift = 1 << (d - 1);  // 2^{d-1}
    uint32_t divisor = 1 << d;      // 2^d
    for (size_t i = 0; i < w.size(); ++i) {
        // w1 = floor((w + 2^{d-1}) / 2^d)
        uint64_t temp = static_cast<uint64_t>(w[i]) + shift;
        w1[i] = temp / divisor;
    }
}

void sample_challenge(std::vector<uint32_t>& c, const std::vector<uint8_t>& seed, uint32_t tau, uint32_t n, uint32_t q) {
    // Initialize SHAKE256 with seed
    SHAKE256Sampler sampler;
    sampler.init(seed.data(), seed.size());

    // Sample tau positions uniformly
    std::vector<uint32_t> positions(n);
    for (uint32_t i = 0; i < n; ++i) {
        positions[i] = i;
    }

    // Fisher-Yates shuffle to select tau positions
    for (uint32_t i = 0; i < tau; ++i) {
        uint32_t j = i + (sampler.sample_uniform(n - i));
        std::swap(positions[i], positions[j]);
    }

    // Set coefficients to 0 initially
    std::fill(c.begin(), c.end(), 0);

    // Assign +1 or -1 to selected positions
    for (uint32_t i = 0; i < tau; ++i) {
        uint8_t sign_byte;
        sampler.squeeze(&sign_byte, 1);
        c[positions[i]] = (sign_byte & 1) ? 1 : (q - 1);  // 1 or -1 mod q
    }
}

bool is_power_of_two(uint32_t x) {
    return (x & (x - 1)) == 0 && x != 0;
}

// Pack polynomial vector into bytes (little-endian 32-bit per coefficient)
std::vector<uint8_t> pack_polynomial_vector(const std::vector<std::vector<uint32_t>>& poly_vector) {
    size_t total_coeffs = 0;
    for (const auto& poly : poly_vector) {
        total_coeffs += poly.size();
    }
    std::vector<uint8_t> packed(total_coeffs * 4);

    size_t offset = 0;
    for (const auto& poly : poly_vector) {
        for (uint32_t coeff : poly) {
            packed[offset++] = coeff & 0xFF;
            packed[offset++] = (coeff >> 8) & 0xFF;
            packed[offset++] = (coeff >> 16) & 0xFF;
            packed[offset++] = (coeff >> 24) & 0xFF;
        }
    }
    return packed;
}

// Unpack bytes into polynomial vector (little-endian 32-bit per coefficient)
std::vector<std::vector<uint32_t>> unpack_polynomial_vector(const std::vector<uint8_t>& data, uint32_t k, uint32_t n) {
    if (data.size() != k * n * 4) {
        throw std::invalid_argument("Data size does not match expected polynomial vector size");
    }

    std::vector<std::vector<uint32_t>> poly_vector(k, std::vector<uint32_t>(n));
    size_t offset = 0;
    for (uint32_t i = 0; i < k; ++i) {
        for (uint32_t j = 0; j < n; ++j) {
            uint32_t coeff = data[offset] |
                             (data[offset + 1] << 8) |
                             (data[offset + 2] << 16) |
                             (data[offset + 3] << 24);
            offset += 4;
            poly_vector[i][j] = coeff;
        }
    }
    return poly_vector;
}

} // namespace clwe