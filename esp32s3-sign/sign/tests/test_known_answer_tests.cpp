#include <gtest/gtest.h>
#include "clwe/color_kem.hpp"
#include "clwe/clwe.hpp"
#include <vector>
#include <array>
#include <iostream>

namespace clwe {

class ColorKEMKnownAnswerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test with different security levels
        security_levels_ = {512, 768, 1024};
    }

    std::vector<uint32_t> security_levels_;
};

// Fixed seeds for deterministic testing
const std::array<uint8_t, 32> MATRIX_SEED_512 = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00
};

const std::array<uint8_t, 32> SECRET_SEED_512 = {
    0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
    0x0f, 0xed, 0xcb, 0xa9, 0x87, 0x65, 0x43, 0x21,
    0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11
};

const std::array<uint8_t, 32> ERROR_SEED_512 = {
    0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01,
    0x10, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32,
    0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22
};

const std::array<uint8_t, 32> R_SEED_512 = {
    0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12,
    0x21, 0x0f, 0xed, 0xcb, 0xa9, 0x87, 0x65, 0x43,
    0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
    0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33
};

const std::array<uint8_t, 32> E1_SEED_512 = {
    0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23,
    0x32, 0x10, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54,
    0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
    0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44
};

const std::array<uint8_t, 32> E2_SEED_512 = {
    0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12, 0x34,
    0x43, 0x21, 0x0f, 0xed, 0xcb, 0xa9, 0x87, 0x65,
    0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd,
    0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55
};

// Test deterministic key generation
TEST_F(ColorKEMKnownAnswerTest, DeterministicKeyGeneration512) {
    CLWEParameters params(512);
    ColorKEM kem(params);

    auto keypair = kem.keygen_deterministic(
        MATRIX_SEED_512, SECRET_SEED_512, ERROR_SEED_512);
    auto public_key = keypair.first;
    auto private_key = keypair.second;

    // Verify parameters
    ASSERT_EQ(public_key.params.security_level, 512u);
    ASSERT_EQ(private_key.params.security_level, 512u);

    // Verify key sizes
    ASSERT_EQ(public_key.public_data.size(), params.module_rank * params.degree * 4);
    ASSERT_EQ(private_key.secret_data.size(), params.module_rank * params.degree * 4);

    // Verify matrix seed is stored
    ASSERT_EQ(public_key.seed, MATRIX_SEED_512);

    // Test serialization/deserialization consistency
    auto pk_serialized = public_key.serialize();
    auto sk_serialized = private_key.serialize();

    auto pk_deserialized = ColorPublicKey::deserialize(pk_serialized, params);
    auto sk_deserialized = ColorPrivateKey::deserialize(sk_serialized, params);

    ASSERT_EQ(pk_deserialized.seed, public_key.seed);
    ASSERT_EQ(pk_deserialized.public_data, public_key.public_data);
    ASSERT_EQ(sk_deserialized.secret_data, private_key.secret_data);
}

// Test deterministic encapsulation
TEST_F(ColorKEMKnownAnswerTest, DeterministicEncapsulation512) {
    CLWEParameters params(512);
    ColorKEM kem(params);

    // Generate deterministic keypair
    auto keypair = kem.keygen_deterministic(
        MATRIX_SEED_512, SECRET_SEED_512, ERROR_SEED_512);
    auto public_key = keypair.first;
    auto private_key = keypair.second;

    // Fixed shared secret for testing
    ColorValue shared_secret = ColorValue::from_math_value(1);

    // Perform deterministic encapsulation
    auto encap_result = kem.encapsulate_deterministic(
        public_key, R_SEED_512, E1_SEED_512, E2_SEED_512, shared_secret);
    auto ciphertext = encap_result.first;
    auto returned_secret = encap_result.second;

    // Verify returned secret matches input
    ASSERT_EQ(returned_secret, shared_secret);

    // Verify ciphertext parameters
    ASSERT_EQ(ciphertext.params.security_level, 512u);
    ASSERT_EQ(ciphertext.ciphertext_data.size(), (params.module_rank + 1) * params.degree * 4);
    ASSERT_EQ(ciphertext.shared_secret_hint.size(), 4);

    // Test serialization/deserialization consistency
    auto ct_serialized = ciphertext.serialize();
    auto ct_deserialized = ColorCiphertext::deserialize(ct_serialized);

    ASSERT_EQ(ct_deserialized.ciphertext_data, ciphertext.ciphertext_data);
    ASSERT_EQ(ct_deserialized.shared_secret_hint, ciphertext.shared_secret_hint);
}

// Test deterministic decapsulation
TEST_F(ColorKEMKnownAnswerTest, DeterministicDecapsulation512) {
    CLWEParameters params(512);
    ColorKEM kem(params);

    // Generate deterministic keypair
    auto keypair = kem.keygen_deterministic(
        MATRIX_SEED_512, SECRET_SEED_512, ERROR_SEED_512);
    auto public_key = keypair.first;
    auto private_key = keypair.second;

    // Fixed shared secret for testing
    ColorValue shared_secret = ColorValue::from_math_value(1);

    // Perform deterministic encapsulation
    auto encap_result = kem.encapsulate_deterministic(
        public_key, R_SEED_512, E1_SEED_512, E2_SEED_512, shared_secret);
    auto ciphertext = encap_result.first;

    // Perform decapsulation
    ColorValue recovered_secret = kem.decapsulate(public_key, private_key, ciphertext);

    // Verify shared secret recovery
    ASSERT_EQ(recovered_secret, shared_secret);
}

// Test full deterministic round-trip
TEST_F(ColorKEMKnownAnswerTest, FullDeterministicRoundTrip512) {
    CLWEParameters params(512);
    ColorKEM kem(params);

    // Generate deterministic keypair
    auto keypair = kem.keygen_deterministic(
        MATRIX_SEED_512, SECRET_SEED_512, ERROR_SEED_512);
    auto public_key = keypair.first;
    auto private_key = keypair.second;

    // Fixed shared secret for testing
    ColorValue shared_secret = ColorValue::from_math_value(1);

    // Encapsulate deterministically
    auto encap_result = kem.encapsulate_deterministic(
        public_key, R_SEED_512, E1_SEED_512, E2_SEED_512, shared_secret);
    auto ciphertext = encap_result.first;

    // Decapsulate
    ColorValue recovered_secret = kem.decapsulate(public_key, private_key, ciphertext);

    // Verify
    ASSERT_EQ(recovered_secret, shared_secret);

    // Test with serialized/deserialized keys
    auto pk_serialized = public_key.serialize();
    auto sk_serialized = private_key.serialize();
    auto ct_serialized = ciphertext.serialize();

    auto pk_deserialized = ColorPublicKey::deserialize(pk_serialized, params);
    auto sk_deserialized = ColorPrivateKey::deserialize(sk_serialized, params);
    auto ct_deserialized = ColorCiphertext::deserialize(ct_serialized);

    ColorValue recovered_from_deserialized = kem.decapsulate(
        pk_deserialized, sk_deserialized, ct_deserialized);

    ASSERT_EQ(recovered_from_deserialized, shared_secret);
}

// Test with different shared secret values
TEST_F(ColorKEMKnownAnswerTest, DifferentSharedSecrets512) {
    CLWEParameters params(512);
    ColorKEM kem(params);

    auto keypair = kem.keygen_deterministic(
        MATRIX_SEED_512, SECRET_SEED_512, ERROR_SEED_512);
    auto public_key = keypair.first;
    auto private_key = keypair.second;

    // Test with shared secret 0
    ColorValue ss0 = ColorValue::from_math_value(0);
    auto encap0 = kem.encapsulate_deterministic(
        public_key, R_SEED_512, E1_SEED_512, E2_SEED_512, ss0);
    auto ct0 = encap0.first;
    ColorValue recovered0 = kem.decapsulate(public_key, private_key, ct0);
    ASSERT_EQ(recovered0, ss0);

    // Test with shared secret 1
    ColorValue ss1 = ColorValue::from_math_value(1);
    auto encap1 = kem.encapsulate_deterministic(
        public_key, R_SEED_512, E1_SEED_512, E2_SEED_512, ss1);
    auto ct1 = encap1.first;
    ColorValue recovered1 = kem.decapsulate(public_key, private_key, ct1);
    ASSERT_EQ(recovered1, ss1);

    // Verify different ciphertexts for different secrets
    ASSERT_NE(ct0.ciphertext_data, ct1.ciphertext_data);
}

// Test for security level 768
TEST_F(ColorKEMKnownAnswerTest, DeterministicKeyGeneration768) {
    CLWEParameters params(768);
    ColorKEM kem(params);

    auto keypair = kem.keygen_deterministic(
        MATRIX_SEED_512, SECRET_SEED_512, ERROR_SEED_512);
    auto public_key = keypair.first;
    auto private_key = keypair.second;

    ASSERT_EQ(public_key.params.security_level, 768u);
    ASSERT_EQ(private_key.params.security_level, 768u);
    ASSERT_EQ(public_key.public_data.size(), params.module_rank * params.degree * 4);
    ASSERT_EQ(private_key.secret_data.size(), params.module_rank * params.degree * 4);
}

// Test for security level 1024
TEST_F(ColorKEMKnownAnswerTest, DeterministicKeyGeneration1024) {
    CLWEParameters params(1024);
    ColorKEM kem(params);

    auto keypair = kem.keygen_deterministic(
        MATRIX_SEED_512, SECRET_SEED_512, ERROR_SEED_512);
    auto public_key = keypair.first;
    auto private_key = keypair.second;

    ASSERT_EQ(public_key.params.security_level, 1024u);
    ASSERT_EQ(private_key.params.security_level, 1024u);
    ASSERT_EQ(public_key.public_data.size(), params.module_rank * params.degree * 4);
    ASSERT_EQ(private_key.secret_data.size(), params.module_rank * params.degree * 4);
}

} // namespace clwe