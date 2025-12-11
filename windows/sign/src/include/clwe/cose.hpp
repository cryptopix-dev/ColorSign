#ifndef CLWE_COSE_HPP
#define CLWE_COSE_HPP

#include "parameters.hpp"
#include "sign.hpp"
#include <vector>
#include <array>
#include <cstdint>

namespace clwe {

// Forward declarations
struct ColorSignature;

// COSE_Sign1 structure
struct COSE_Sign1 {
    std::vector<uint8_t> protected_header;  // CBOR-encoded protected header
    std::vector<uint8_t> unprotected_header; // CBOR-encoded unprotected header (usually empty)
    std::vector<uint8_t> payload;           // Message payload (can be nil)
    std::vector<uint8_t> signature;         // Signature bytes

    COSE_Sign1() = default;
    COSE_Sign1(const std::vector<uint8_t>& prot, const std::vector<uint8_t>& unprot,
               const std::vector<uint8_t>& pay, const std::vector<uint8_t>& sig)
        : protected_header(prot), unprotected_header(unprot), payload(pay), signature(sig) {}
};

// COSE Header structure
struct COSE_Header {
    int alg;  // Algorithm identifier
    // Add other headers as needed
};

// Encode COSE_Sign1 to CBOR bytes
std::vector<uint8_t> encode_cose_sign1(const COSE_Sign1& cose_msg);

// Decode COSE_Sign1 from CBOR bytes
COSE_Sign1 decode_cose_sign1(const std::vector<uint8_t>& cbor_data);

// Encode COSE header to CBOR
std::vector<uint8_t> encode_cose_header(const COSE_Header& header);

// Decode COSE header from CBOR
COSE_Header decode_cose_header(const std::vector<uint8_t>& cbor_data);

// Create COSE_Sign1 from ColorSignature and message
COSE_Sign1 create_cose_sign1_from_colorsign(const std::vector<uint8_t>& message,
                                           const ColorSignature& signature,
                                           int alg = COSE_ALG_ML_DSA_44);

// Extract ColorSignature from COSE_Sign1
ColorSignature extract_colorsign_from_cose(const COSE_Sign1& cose_msg,
                                          const CLWEParameters& params);

// CBOR encoding utilities
namespace cbor {

// CBOR major types
enum MajorType {
    UNSIGNED_INT = 0,
    NEGATIVE_INT = 1,
    BYTE_STRING = 2,
    TEXT_STRING = 3,
    ARRAY = 4,
    MAP = 5,
    TAG = 6,
    SIMPLE_VALUE = 7
};

// Encode unsigned integer
std::vector<uint8_t> encode_uint(uint64_t value);

// Encode byte string
std::vector<uint8_t> encode_bstr(const std::vector<uint8_t>& data);

// Encode array
std::vector<uint8_t> encode_array(const std::vector<std::vector<uint8_t>>& items);

// Encode map (simple key-value pairs, keys as ints)
std::vector<uint8_t> encode_map(const std::vector<std::pair<int, std::vector<uint8_t>>>& pairs);

// Decode functions (basic)
uint64_t decode_uint(const std::vector<uint8_t>& data, size_t& offset);
std::vector<uint8_t> decode_bstr(const std::vector<uint8_t>& data, size_t& offset);
std::vector<std::vector<uint8_t>> decode_array(const std::vector<uint8_t>& data, size_t& offset);

} // namespace cbor

} // namespace clwe

#endif // CLWE_COSE_HPP