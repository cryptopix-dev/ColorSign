#include "../include/clwe/cose.hpp"
#include "../include/clwe/sign.hpp"
#include <stdexcept>
#include <algorithm>

namespace clwe {

// CBOR encoding utilities implementation
namespace cbor {

std::vector<uint8_t> encode_uint(uint64_t value) {
    std::vector<uint8_t> result;
    if (value < 24) {
        result.push_back(static_cast<uint8_t>(UNSIGNED_INT << 5) | static_cast<uint8_t>(value));
    } else if (value <= 0xFF) {
        result.push_back(static_cast<uint8_t>(UNSIGNED_INT << 5) | 24);
        result.push_back(static_cast<uint8_t>(value));
    } else if (value <= 0xFFFF) {
        result.push_back(static_cast<uint8_t>(UNSIGNED_INT << 5) | 25);
        result.push_back(static_cast<uint8_t>(value >> 8));
        result.push_back(static_cast<uint8_t>(value & 0xFF));
    } else if (value <= 0xFFFFFFFF) {
        result.push_back(static_cast<uint8_t>(UNSIGNED_INT << 5) | 26);
        for (int i = 24; i >= 0; i -= 8) {
            result.push_back(static_cast<uint8_t>((value >> i) & 0xFF));
        }
    } else {
        result.push_back(static_cast<uint8_t>(UNSIGNED_INT << 5) | 27);
        for (int i = 56; i >= 0; i -= 8) {
            result.push_back(static_cast<uint8_t>((value >> i) & 0xFF));
        }
    }
    return result;
}

std::vector<uint8_t> encode_bstr(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> result;
    size_t len = data.size();
    if (len < 24) {
        result.push_back(static_cast<uint8_t>(BYTE_STRING << 5) | static_cast<uint8_t>(len));
    } else if (len <= 0xFF) {
        result.push_back(static_cast<uint8_t>(BYTE_STRING << 5) | 24);
        result.push_back(static_cast<uint8_t>(len));
    } else if (len <= 0xFFFF) {
        result.push_back(static_cast<uint8_t>(BYTE_STRING << 5) | 25);
        result.push_back(static_cast<uint8_t>(len >> 8));
        result.push_back(static_cast<uint8_t>(len & 0xFF));
    } else {
        // For simplicity, assume len <= 0xFFFFFFFF
        result.push_back(static_cast<uint8_t>(BYTE_STRING << 5) | 26);
        for (int i = 24; i >= 0; i -= 8) {
            result.push_back(static_cast<uint8_t>((len >> i) & 0xFF));
        }
    }
    result.insert(result.end(), data.begin(), data.end());
    return result;
}

std::vector<uint8_t> encode_array(const std::vector<std::vector<uint8_t>>& items) {
    std::vector<uint8_t> result;
    size_t len = items.size();
    if (len < 24) {
        result.push_back(static_cast<uint8_t>(ARRAY << 5) | static_cast<uint8_t>(len));
    } else {
        // For simplicity, assume small arrays
        result.push_back(static_cast<uint8_t>(ARRAY << 5) | 24);
        result.push_back(static_cast<uint8_t>(len));
    }
    for (const auto& item : items) {
        result.insert(result.end(), item.begin(), item.end());
    }
    return result;
}

std::vector<uint8_t> encode_map(const std::vector<std::pair<int, std::vector<uint8_t>>>& pairs) {
    std::vector<uint8_t> result;
    size_t len = pairs.size();
    if (len < 24) {
        result.push_back(static_cast<uint8_t>(MAP << 5) | static_cast<uint8_t>(len));
    } else {
        // Assume small maps
        result.push_back(static_cast<uint8_t>(MAP << 5) | 24);
        result.push_back(static_cast<uint8_t>(len));
    }
    for (const auto& pair : pairs) {
        auto key_encoded = encode_uint(pair.first);
        result.insert(result.end(), key_encoded.begin(), key_encoded.end());
        result.insert(result.end(), pair.second.begin(), pair.second.end());
    }
    return result;
}

// Basic decode functions (simplified)
uint64_t decode_uint(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset >= data.size()) throw std::invalid_argument("CBOR decode: out of bounds");
    uint8_t initial = data[offset++];
    uint8_t major = initial >> 5;
    uint8_t minor = initial & 0x1F;
    if (major != UNSIGNED_INT) throw std::invalid_argument("CBOR decode: not unsigned int");
    if (minor < 24) return minor;
    if (minor == 24) {
        if (offset >= data.size()) throw std::invalid_argument("CBOR decode: incomplete uint");
        return data[offset++];
    }
    // Simplified, assume no larger
    throw std::invalid_argument("CBOR decode: large uint not supported");
}

std::vector<uint8_t> decode_bstr(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset >= data.size()) throw std::invalid_argument("CBOR decode: out of bounds");
    uint8_t initial = data[offset++];
    uint8_t major = initial >> 5;
    uint8_t minor = initial & 0x1F;
    if (major != BYTE_STRING) throw std::invalid_argument("CBOR decode: not byte string");
    size_t len;
    if (minor < 24) len = minor;
    else if (minor == 24) {
        if (offset >= data.size()) throw std::invalid_argument("CBOR decode: incomplete bstr len");
        len = data[offset++];
    } else {
        throw std::invalid_argument("CBOR decode: large bstr not supported");
    }
    if (offset + len > data.size()) throw std::invalid_argument("CBOR decode: bstr data incomplete");
    std::vector<uint8_t> result(data.begin() + offset, data.begin() + offset + len);
    offset += len;
    return result;
}

std::vector<std::vector<uint8_t>> decode_array(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset >= data.size()) throw std::invalid_argument("CBOR decode: out of bounds");
    uint8_t initial = data[offset++];
    uint8_t major = initial >> 5;
    uint8_t minor = initial & 0x1F;
    if (major != ARRAY) throw std::invalid_argument("CBOR decode: not array");
    size_t len;
    if (minor < 24) len = minor;
    else if (minor == 24) {
        if (offset >= data.size()) throw std::invalid_argument("CBOR decode: incomplete array len");
        len = data[offset++];
    } else {
        throw std::invalid_argument("CBOR decode: large array not supported");
    }
    std::vector<std::vector<uint8_t>> result;
    for (size_t i = 0; i < len; ++i) {
        result.push_back(decode_bstr(data, offset));  // Assume all bstr for COSE_Sign1
    }
    return result;
}

} // namespace cbor

// Encode COSE header to CBOR
std::vector<uint8_t> encode_cose_header(const COSE_Header& header) {
    std::vector<std::pair<int, std::vector<uint8_t>>> pairs;
    pairs.emplace_back(1, cbor::encode_uint(header.alg));  // alg key is 1
    return cbor::encode_map(pairs);
}

// Decode COSE header from CBOR
COSE_Header decode_cose_header(const std::vector<uint8_t>& cbor_data) {
    // Simplified: assume map with one pair {1: alg}
    size_t offset = 0;
    if (offset >= cbor_data.size()) throw std::invalid_argument("Empty CBOR data");
    uint8_t initial = cbor_data[offset++];
    if ((initial >> 5) != cbor::MAP) throw std::invalid_argument("Not a map");
    uint8_t len = initial & 0x1F;
    if (len != 1) throw std::invalid_argument("Expected map of length 1");
    uint64_t key = cbor::decode_uint(cbor_data, offset);
    if (key != 1) throw std::invalid_argument("Expected alg key");
    uint64_t alg = cbor::decode_uint(cbor_data, offset);
    COSE_Header header;
    header.alg = static_cast<int>(alg);
    return header;
}

// Encode COSE_Sign1 to CBOR
std::vector<uint8_t> encode_cose_sign1(const COSE_Sign1& cose_msg) {
    std::vector<std::vector<uint8_t>> items;
    items.push_back(cbor::encode_bstr(cose_msg.protected_header));
    items.push_back(cbor::encode_bstr(cose_msg.unprotected_header));
    items.push_back(cbor::encode_bstr(cose_msg.payload));
    items.push_back(cbor::encode_bstr(cose_msg.signature));
    return cbor::encode_array(items);
}

// Decode COSE_Sign1 from CBOR
COSE_Sign1 decode_cose_sign1(const std::vector<uint8_t>& cbor_data) {
    size_t offset = 0;
    auto items = cbor::decode_array(cbor_data, offset);
    if (items.size() != 4) throw std::invalid_argument("COSE_Sign1 must have 4 elements");
    COSE_Sign1 cose;
    cose.protected_header = items[0];
    cose.unprotected_header = items[1];
    cose.payload = items[2];
    cose.signature = items[3];
    return cose;
}

// Create COSE_Sign1 from ColorSignature and message
COSE_Sign1 create_cose_sign1_from_colorsign(const std::vector<uint8_t>& message,
                                           const ColorSignature& signature,
                                           int alg) {
    COSE_Header header;
    header.alg = alg;
    std::vector<uint8_t> protected_cbor = encode_cose_header(header);
    std::vector<uint8_t> unprotected_cbor = cbor::encode_map({});  // Empty map
    std::vector<uint8_t> payload = message;
    std::vector<uint8_t> sig_bytes = signature.serialize();
    return COSE_Sign1(protected_cbor, unprotected_cbor, payload, sig_bytes);
}

// Extract ColorSignature from COSE_Sign1
ColorSignature extract_colorsign_from_cose(const COSE_Sign1& cose_msg,
                                          const CLWEParameters& params) {
    return ColorSignature::deserialize(cose_msg.signature, params);
}

} // namespace clwe