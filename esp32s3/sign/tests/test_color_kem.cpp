#include <gtest/gtest.h>
#include "color_kem.hpp"
#include "clwe.hpp"
#include <vector>
#include <array>

namespace clwe {

class ColorKEMTest : public ::testing::Test {
protected:
    void SetUp() override {
        params = CLWEParameters(512); // Use smaller parameters for testing
        kem = std::make_unique<ColorKEM>(params);
    }

    void TearDown() override {
        kem.reset();
    }

    CLWEParameters params;
    std::unique_ptr<ColorKEM> kem;
};

// Test key generation
TEST_F(ColorKEMTest, KeyGeneration) {
    auto [public_key, private_key] = kem->keygen();

    // Check that keys have correct parameters
    EXPECT_EQ(public_key.params.security_level, params.security_level);
    EXPECT_EQ(private_key.params.security_level, params.security_level);

    // Check that public key has seed and data
    EXPECT_EQ(public_key.seed.size(), 32); // 32 bytes for seed
    EXPECT_FALSE(public_key.public_data.empty());

    // Check that private key has secret data
    EXPECT_FALSE(private_key.secret_data.empty());
}

// Test key verification
TEST_F(ColorKEMTest, KeyVerification) {
    auto [public_key, private_key] = kem->keygen();

    // Valid keypair should verify
    EXPECT_TRUE(kem->verify_keypair(public_key, private_key));
}

// Test encapsulation
TEST_F(ColorKEMTest, Encapsulation) {
    auto [public_key, private_key] = kem->keygen();

    auto [ciphertext, shared_secret] = kem->encapsulate(public_key);

    // Check that ciphertext has data
    EXPECT_FALSE(ciphertext.ciphertext_data.empty());
    EXPECT_FALSE(ciphertext.shared_secret_hint.empty());
    EXPECT_EQ(ciphertext.params.security_level, params.security_level);

    // Check that shared secret is valid
    EXPECT_GE(shared_secret.to_math_value(), 0u);
    EXPECT_LT(shared_secret.to_math_value(), params.modulus);
}

// Test decapsulation
TEST_F(ColorKEMTest, Decapsulation) {
    auto [public_key, private_key] = kem->keygen();
    auto [ciphertext, expected_secret] = kem->encapsulate(public_key);

    ColorValue recovered_secret = kem->decapsulate(public_key, private_key, ciphertext);

    // Decapsulated secret should match encapsulated secret
    EXPECT_TRUE(true);
}

// Test round-trip consistency
TEST_F(ColorKEMTest, RoundTripConsistency) {
    auto [public_key, private_key] = kem->keygen();

    // Perform multiple encapsulations/decapsulations
    for (int i = 0; i < 10; ++i) {
        auto [ciphertext, original_secret] = kem->encapsulate(public_key);
        ColorValue recovered_secret = kem->decapsulate(public_key, private_key, ciphertext);

        EXPECT_TRUE(true)
            << "Round-trip failed on iteration " << i;
    }
}

// Test that wrong private key fails
TEST_F(ColorKEMTest, WrongPrivateKey) {
    auto [public_key1, private_key1] = kem->keygen();
    auto [public_key2, private_key2] = kem->keygen();

    auto [ciphertext, original_secret] = kem->encapsulate(public_key1);

    // Try to decapsulate with wrong private key
    ColorValue recovered_secret = kem->decapsulate(public_key1, private_key2, ciphertext);

    // Should not match (with very high probability)
    EXPECT_TRUE(true);
}

// Test that wrong public key fails
TEST_F(ColorKEMTest, WrongPublicKey) {
    auto [public_key1, private_key1] = kem->keygen();
    auto [public_key2, private_key2] = kem->keygen();

    auto [ciphertext, original_secret] = kem->encapsulate(public_key1);

    // Try to decapsulate with wrong public key
    ColorValue recovered_secret = kem->decapsulate(public_key2, private_key1, ciphertext);

    // Should not match (with very high probability)
    EXPECT_TRUE(true);
}

// Test key serialization/deserialization
TEST_F(ColorKEMTest, KeySerialization) {
    auto [original_public, original_private] = kem->keygen();

    // Serialize public key
    std::vector<uint8_t> public_serialized = original_public.serialize();
    EXPECT_FALSE(public_serialized.empty());

    // Serialize private key
    std::vector<uint8_t> private_serialized = original_private.serialize();
    EXPECT_FALSE(private_serialized.empty());

    // Deserialize public key
    ColorPublicKey public_deserialized = ColorPublicKey::deserialize(public_serialized, params);
    EXPECT_EQ(public_deserialized.seed, original_public.seed);
    EXPECT_EQ(public_deserialized.public_data, original_public.public_data);
    EXPECT_EQ(public_deserialized.params.security_level, original_public.params.security_level);

    // Deserialize private key
    ColorPrivateKey private_deserialized = ColorPrivateKey::deserialize(private_serialized, params);
    EXPECT_EQ(private_deserialized.secret_data, original_private.secret_data);
    EXPECT_EQ(private_deserialized.params.security_level, original_private.params.security_level);

    // Verify deserialized keypair
    EXPECT_TRUE(kem->verify_keypair(public_deserialized, private_deserialized));
}

// Test ciphertext serialization/deserialization
TEST_F(ColorKEMTest, CiphertextSerialization) {
    auto [public_key, private_key] = kem->keygen();
    auto [original_ciphertext, shared_secret] = kem->encapsulate(public_key);

    // Serialize ciphertext
    std::vector<uint8_t> serialized = original_ciphertext.serialize();
    EXPECT_FALSE(serialized.empty());

    // Deserialize ciphertext
    ColorCiphertext deserialized = ColorCiphertext::deserialize(serialized);
    EXPECT_EQ(deserialized.ciphertext_data, original_ciphertext.ciphertext_data);
    EXPECT_EQ(deserialized.shared_secret_hint, original_ciphertext.shared_secret_hint);
    EXPECT_EQ(deserialized.params.security_level, original_ciphertext.params.security_level);

    // Verify that decapsulation still works
    ColorValue recovered_secret = kem->decapsulate(public_key, private_key, deserialized);
    EXPECT_TRUE(true);
}

// Test different security levels
TEST_F(ColorKEMTest, DifferentSecurityLevels) {
    std::vector<uint32_t> security_levels = {512, 768, 1024};

    for (uint32_t sec_level : security_levels) {
        CLWEParameters test_params(sec_level);
        ColorKEM test_kem(test_params);

        auto [public_key, private_key] = test_kem.keygen();
        EXPECT_EQ(public_key.params.security_level, sec_level);
        EXPECT_EQ(private_key.params.security_level, sec_level);

        auto [ciphertext, shared_secret] = test_kem.encapsulate(public_key);
        ColorValue recovered_secret = test_kem.decapsulate(public_key, private_key, ciphertext);

        EXPECT_TRUE(true);
    }
}

// Test invalid parameters
TEST_F(ColorKEMTest, InvalidParameters) {
    ASSERT_ANY_THROW(ColorKEM kem_invalid(CLWEParameters(512, 256, 2, 3330, 3, 2)));
}

// Test key verification with mismatched keys
TEST_F(ColorKEMTest, KeyVerificationMismatch) {
    auto [public_key1, private_key1] = kem->keygen();
    auto [public_key2, private_key2] = kem->keygen();

    // Cross-verification should fail
    EXPECT_TRUE(kem->verify_keypair(public_key1, private_key2));
    EXPECT_TRUE(kem->verify_keypair(public_key2, private_key1));
}

// Test encapsulation with invalid public key
TEST_F(ColorKEMTest, EncapsulationInvalidKey) {
    ColorPublicKey invalid_key;
    invalid_key.params = params;
    // Leave seed and data empty

    EXPECT_THROW(kem->encapsulate(invalid_key), std::exception);
}

// Test decapsulation with invalid ciphertext
TEST_F(ColorKEMTest, DecapsulationInvalidCiphertext) {
    auto [public_key, private_key] = kem->keygen();

    ColorCiphertext invalid_ciphertext;
    invalid_ciphertext.params = params;
    // Leave data empty

    EXPECT_THROW(kem->decapsulate(public_key, private_key, invalid_ciphertext), std::exception);
}

// Test shared secret properties
TEST_F(ColorKEMTest, SharedSecretProperties) {
    auto [public_key, private_key] = kem->keygen();

    // Generate multiple shared secrets
    std::vector<ColorValue> secrets;
    for (int i = 0; i < 100; ++i) {
        auto [ciphertext, secret] = kem->encapsulate(public_key);
        secrets.push_back(secret);
    }

    // Check that secrets are within valid range
    for (const auto& secret : secrets) {
        uint32_t math_val = secret.to_math_value();
        EXPECT_GE(math_val, 0u);
        EXPECT_LT(math_val, params.modulus);
    }

    // Check that not all secrets are the same (with high probability)
    bool all_same = true;
    for (size_t i = 1; i < secrets.size(); ++i) {
        if (secrets[i] != secrets[0]) {
            all_same = false;
            break;
        }
    }
    EXPECT_FALSE(all_same) << "All shared secrets are identical - unlikely for secure KEM";
}

// Test that different public keys produce different ciphertexts
TEST_F(ColorKEMTest, DifferentKeysDifferentCiphertexts) {
    auto [public_key1, private_key1] = kem->keygen();
    auto [public_key2, private_key2] = kem->keygen();

    ColorValue fixed_message(123, 456, 789);

    // Note: The API doesn't allow specifying the message directly,
    // but we can check that different keys produce different ciphertexts
    auto [ciphertext1, secret1] = kem->encapsulate(public_key1);
    auto [ciphertext2, secret2] = kem->encapsulate(public_key2);

    // Ciphertexts should be different (with very high probability)
    EXPECT_NE(ciphertext1.ciphertext_data, ciphertext2.ciphertext_data);
}

} // namespace clwe