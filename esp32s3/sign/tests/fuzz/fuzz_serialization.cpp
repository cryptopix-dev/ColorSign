#include <cstdint>
#include <vector>
#include <array>
#include "clwe/color_kem.hpp"
#include "clwe/clwe.hpp"

namespace clwe {

// Fuzz target for serialization/deserialization with malformed data
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 4) return 0;

    try {
        // Create valid parameters for testing
        CLWEParameters params(512);

        // Test public key deserialization with fuzzed data
        try {
            ColorPublicKey::deserialize(std::vector<uint8_t>(data, data + size), params);
        } catch (const std::exception&) {
            // Expected for malformed data
        }

        // Test private key deserialization with fuzzed data
        try {
            ColorPrivateKey::deserialize(std::vector<uint8_t>(data, data + size));
        } catch (const std::exception&) {
            // Expected for malformed data
        }

        // Test ciphertext deserialization with fuzzed data
        try {
            ColorCiphertext::deserialize(std::vector<uint8_t>(data, data + size));
        } catch (const std::exception&) {
            // Expected for malformed data
        }

        // Test round-trip with valid data but corrupted during fuzzing
        if (size > 10) {
            try {
                ColorKEM kem(params);
                auto keypair = kem.keygen();
                auto pub_key = keypair.first;
                auto priv_key = keypair.second;

                // Serialize
                std::vector<uint8_t> pub_ser = pub_key.serialize();
                std::vector<uint8_t> priv_ser = priv_key.serialize();

                // Corrupt the serialized data with fuzz input
                if (pub_ser.size() > 0 && size > 0) {
                    size_t corrupt_pos = (data[0] * pub_ser.size()) / 256;
                    if (corrupt_pos < pub_ser.size()) {
                        pub_ser[corrupt_pos] ^= data[1];
                        try {
                            ColorPublicKey::deserialize(pub_ser, params);
                        } catch (const std::exception&) {
                            // Expected
                        }
                    }
                }

                if (priv_ser.size() > 0 && size > 2) {
                    size_t corrupt_pos = (data[2] * priv_ser.size()) / 256;
                    if (corrupt_pos < priv_ser.size()) {
                        priv_ser[corrupt_pos] ^= data[3];
                        try {
                            ColorPrivateKey::deserialize(priv_ser);
                        } catch (const std::exception&) {
                            // Expected
                        }
                    }
                }

                // Test encapsulation and corrupt ciphertext
                auto encap_result = kem.encapsulate(pub_key);
                auto ciphertext = encap_result.first;
                auto secret = encap_result.second;
                std::vector<uint8_t> ct_ser = ciphertext.serialize();

                if (ct_ser.size() > 0 && size > 4) {
                    size_t corrupt_pos = (data[4] * ct_ser.size()) / 256;
                    if (corrupt_pos < ct_ser.size()) {
                        ct_ser[corrupt_pos] ^= data[5];
                        try {
                            ColorCiphertext::deserialize(ct_ser);
                        } catch (const std::exception&) {
                            // Expected
                        }
                    }
                }

            } catch (const std::exception&) {
                // Expected during key generation or operations
            }
        }

        // Test with truncated data
        if (size > 5) {
            try {
                ColorKEM kem(params);
                auto keypair = kem.keygen();
                auto pub_key = keypair.first;
                auto priv_key = keypair.second;

                std::vector<uint8_t> pub_ser = pub_key.serialize();
                if (pub_ser.size() > 5) {
                    // Try deserializing truncated versions
                    for (size_t len = 1; len < pub_ser.size() && len < size; ++len) {
                        try {
                            ColorPublicKey::deserialize(std::vector<uint8_t>(pub_ser.begin(), pub_ser.begin() + len), params);
                        } catch (const std::exception&) {
                            // Expected
                        }
                    }
                }
            } catch (const std::exception&) {
                // Expected
            }
        }

    } catch (const std::exception&) {
        // Catch any unexpected exceptions
        return 0;
    }

    return 0;
}

} // namespace clwe