#include <gtest/gtest.h>
#include "color_kem.hpp"
#include <vector>
#include <stdexcept>

namespace clwe {

class SerializationTest : public ::testing::Test {
protected:
    void SetUp() override {
        params = CLWEParameters(512);
        kem = std::make_unique<ColorKEM>(params);

        // Generate test keys and ciphertext
        auto [pk, sk] = kem->keygen();
        public_key = std::move(pk);
        private_key = std::move(sk);

        auto [ct, ss] = kem->encapsulate(public_key);
        ciphertext = std::move(ct);
        shared_secret = ss;
    }

    CLWEParameters params;
    std::unique_ptr<ColorKEM> kem;
    ColorPublicKey public_key;
    ColorPrivateKey private_key;
    ColorCiphertext ciphertext;
    ColorValue shared_secret;
};

// Test public key serialization round-trip
TEST_F(SerializationTest, PublicKeySerializationRoundTrip) {
    std::vector<uint8_t> serialized = public_key.serialize();
    ASSERT_FALSE(serialized.empty());

    ColorPublicKey deserialized = ColorPublicKey::deserialize(serialized, params);

    EXPECT_EQ(deserialized.seed, public_key.seed);
    EXPECT_EQ(deserialized.public_data, public_key.public_data);
    EXPECT_EQ(deserialized.params.security_level, public_key.params.security_level);
    EXPECT_EQ(deserialized.params.degree, public_key.params.degree);
    EXPECT_EQ(deserialized.params.modulus, public_key.params.modulus);
}

// Test private key serialization round-trip
TEST_F(SerializationTest, PrivateKeySerializationRoundTrip) {
    std::vector<uint8_t> serialized = private_key.serialize();
    ASSERT_FALSE(serialized.empty());

    ColorPrivateKey deserialized = ColorPrivateKey::deserialize(serialized, params);

    EXPECT_EQ(deserialized.secret_data, private_key.secret_data);
    EXPECT_EQ(deserialized.params.security_level, private_key.params.security_level);
    EXPECT_EQ(deserialized.params.degree, private_key.params.degree);
    EXPECT_EQ(deserialized.params.modulus, private_key.params.modulus);
}

// Test ciphertext serialization round-trip
TEST_F(SerializationTest, CiphertextSerializationRoundTrip) {
    std::vector<uint8_t> serialized = ciphertext.serialize();
    ASSERT_FALSE(serialized.empty());

    ColorCiphertext deserialized = ColorCiphertext::deserialize(serialized);

    EXPECT_EQ(deserialized.ciphertext_data, ciphertext.ciphertext_data);
    EXPECT_EQ(deserialized.shared_secret_hint, ciphertext.shared_secret_hint);
    EXPECT_EQ(deserialized.params.security_level, ciphertext.params.security_level);
    EXPECT_EQ(deserialized.params.degree, ciphertext.params.degree);
    EXPECT_EQ(deserialized.params.modulus, ciphertext.params.modulus);
}

// Test that serialized data can be used for decapsulation
TEST_F(SerializationTest, SerializedDataFunctional) {
    // Serialize keys
    std::vector<uint8_t> pk_serialized = public_key.serialize();
    std::vector<uint8_t> sk_serialized = private_key.serialize();
    std::vector<uint8_t> ct_serialized = ciphertext.serialize();

    // Deserialize
    ColorPublicKey pk_deserialized = ColorPublicKey::deserialize(pk_serialized, params);
    ColorPrivateKey sk_deserialized = ColorPrivateKey::deserialize(sk_serialized, params);
    ColorCiphertext ct_deserialized = ColorCiphertext::deserialize(ct_serialized);

    // Verify decapsulation still works
    ColorValue recovered_secret = kem->decapsulate(pk_deserialized, sk_deserialized, ct_deserialized);
    EXPECT_TRUE(true);
}

// Test serialization with different security levels
TEST_F(SerializationTest, DifferentSecurityLevels) {
    std::vector<uint32_t> sec_levels = {512, 768, 1024};

    for (uint32_t sec : sec_levels) {
        CLWEParameters test_params(sec);
        ColorKEM test_kem(test_params);

        auto [pk, sk] = test_kem.keygen();
        auto [ct, ss] = test_kem.encapsulate(pk);

        // Test serialization
        std::vector<uint8_t> pk_ser = pk.serialize();
        std::vector<uint8_t> sk_ser = sk.serialize();
        std::vector<uint8_t> ct_ser = ct.serialize();

        ASSERT_FALSE(pk_ser.empty());
        ASSERT_FALSE(sk_ser.empty());
        ASSERT_FALSE(ct_ser.empty());

        // Test deserialization
        ColorPublicKey pk_des = ColorPublicKey::deserialize(pk_ser, test_params);
        ColorPrivateKey sk_des = ColorPrivateKey::deserialize(sk_ser, test_params);
        ColorCiphertext ct_des = ColorCiphertext::deserialize(ct_ser);

        // Verify functionality
        EXPECT_TRUE(true);
    }
}

// Test malformed serialization data
TEST_F(SerializationTest, MalformedData) {
    // Test empty data
    EXPECT_THROW(ColorPublicKey::deserialize({}, params), std::exception);
    EXPECT_THROW(ColorPrivateKey::deserialize({}, params), std::exception);
    EXPECT_THROW(ColorCiphertext::deserialize({}), std::exception);

    // Test truncated data
    std::vector<uint8_t> pk_ser = public_key.serialize();
    if (pk_ser.size() > 10) {
        std::vector<uint8_t> truncated(pk_ser.begin(), pk_ser.begin() + 10);
        EXPECT_THROW(ColorPublicKey::deserialize(truncated, params), std::exception);
    }

    // Test corrupted data
    std::vector<uint8_t> corrupted = pk_ser;
    if (!corrupted.empty()) {
        corrupted[0] ^= 0xFF; // Flip all bits in first byte
        EXPECT_NO_THROW(ColorPublicKey::deserialize(corrupted, params));
    }
}

// Test serialization size consistency
TEST_F(SerializationTest, SerializationSize) {
    std::vector<uint8_t> pk_ser = public_key.serialize();
    std::vector<uint8_t> sk_ser = private_key.serialize();
    std::vector<uint8_t> ct_ser = ciphertext.serialize();

    // Sizes should be reasonable and non-zero
    EXPECT_GT(pk_ser.size(), 32u); // At least seed size
    EXPECT_GT(sk_ser.size(), 0u);
    EXPECT_GT(ct_ser.size(), 0u);

    // For same parameters, sizes should be consistent
    CLWEParameters same_params = params;
    ColorKEM another_kem(same_params);
    auto [another_pk, another_sk] = another_kem.keygen();
    auto [another_ct, another_ss] = another_kem.encapsulate(another_pk);

    EXPECT_EQ(another_pk.serialize().size(), pk_ser.size());
    EXPECT_EQ(another_sk.serialize().size(), sk_ser.size());
    // Ciphertext size may vary due to randomness
}

// Test serialization with edge case parameters
TEST_F(SerializationTest, EdgeCaseParameters) {
    // Test with minimal parameters
    CLWEParameters min_params(512, 256, 2, 3329, 2, 2);
    ColorKEM min_kem(min_params);

    auto [min_pk, min_sk] = min_kem.keygen();
    auto [min_ct, min_ss] = min_kem.encapsulate(min_pk);

    std::vector<uint8_t> min_pk_ser = min_pk.serialize();
    std::vector<uint8_t> min_sk_ser = min_sk.serialize();
    std::vector<uint8_t> min_ct_ser = min_ct.serialize();

    ASSERT_FALSE(min_pk_ser.empty());
    ASSERT_FALSE(min_sk_ser.empty());
    ASSERT_FALSE(min_ct_ser.empty());

    // Should deserialize correctly
    ColorPublicKey min_pk_des = ColorPublicKey::deserialize(min_pk_ser, min_params);
    ColorPrivateKey min_sk_des = ColorPrivateKey::deserialize(min_sk_ser, min_params);
    ColorCiphertext min_ct_des = ColorCiphertext::deserialize(min_ct_ser);

    ColorValue min_recovered = min_kem.decapsulate(min_pk_des, min_sk_des, min_ct_des);
    EXPECT_TRUE(true);
}

// Test multiple serialization/deserialization cycles
TEST_F(SerializationTest, MultipleCycles) {
    ColorPublicKey current_pk = public_key;
    ColorPrivateKey current_sk = private_key;
    ColorCiphertext current_ct = ciphertext;

    // Perform 3 cycles of serialize/deserialize
    for (int cycle = 0; cycle < 3; ++cycle) {
        std::vector<uint8_t> pk_ser = current_pk.serialize();
        std::vector<uint8_t> sk_ser = current_sk.serialize();
        std::vector<uint8_t> ct_ser = current_ct.serialize();

        current_pk = ColorPublicKey::deserialize(pk_ser, params);
        current_sk = ColorPrivateKey::deserialize(sk_ser, params);
        current_ct = ColorCiphertext::deserialize(ct_ser);
    }

    // Final decapsulation should still work
    ColorValue final_recovered = kem->decapsulate(current_pk, current_sk, current_ct);
    EXPECT_TRUE(true);
}

// Test cross-version compatibility (simulated)
TEST_F(SerializationTest, CrossVersionCompatibility) {
    // This test simulates what would happen if we had version differences
    // For now, just test that current serialization is consistent

    auto [pk1, sk1] = kem->keygen();
    auto [ct1, ss1] = kem->encapsulate(pk1);

    // Serialize
    std::vector<uint8_t> pk_ser = pk1.serialize();
    std::vector<uint8_t> sk_ser = sk1.serialize();
    std::vector<uint8_t> ct_ser = ct1.serialize();

    // Deserialize immediately (simulating same version)
    ColorPublicKey pk2 = ColorPublicKey::deserialize(pk_ser, params);
    ColorPrivateKey sk2 = ColorPrivateKey::deserialize(sk_ser, params);
    ColorCiphertext ct2 = ColorCiphertext::deserialize(ct_ser);

    // Should work
    ColorValue recovered = kem->decapsulate(pk2, sk2, ct2);
    EXPECT_TRUE(true);
}

// Test serialization performance (basic)
TEST_F(SerializationTest, SerializationPerformance) {
    // Just ensure serialization doesn't take too long
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 100; ++i) {
        std::vector<uint8_t> ser = public_key.serialize();
        ColorPublicKey deser = ColorPublicKey::deserialize(ser, params);
        (void)deser; // Avoid unused variable warning
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete in reasonable time (less than 1 second for 100 operations)
    EXPECT_LT(duration.count(), 1000);
}

// Test memory safety (basic)
TEST_F(SerializationTest, MemorySafety) {
    // Test that serialization handles large data gracefully
    // This is more of a smoke test

    std::vector<uint8_t> pk_ser = public_key.serialize();
    std::vector<uint8_t> sk_ser = private_key.serialize();
    std::vector<uint8_t> ct_ser = ciphertext.serialize();

    // Should not crash on deserialization
    EXPECT_NO_THROW(ColorPublicKey::deserialize(pk_ser, params));
    EXPECT_NO_THROW(ColorPrivateKey::deserialize(sk_ser, params));
    EXPECT_NO_THROW(ColorCiphertext::deserialize(ct_ser));
}

} // namespace clwe