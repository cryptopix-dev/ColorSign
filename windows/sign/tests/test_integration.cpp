#include <gtest/gtest.h>
#include "keygen.hpp"
#include "sign.hpp"
#include "verify.hpp"
#include <vector>
#include <string>

namespace {

// Integration test fixture
class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test with all security levels
        params44 = clwe::CLWEParameters(44);
        params65 = clwe::CLWEParameters(65);
        params87 = clwe::CLWEParameters(87);
    }

    clwe::CLWEParameters params44, params65, params87;
};

TEST_F(IntegrationTest, FullSignVerifyCycle44) {
    // Generate keypair
    clwe::ColorSignKeyGen keygen(params44);
    auto [public_key, private_key] = keygen.generate_keypair();

    // Create signer and verifier
    clwe::ColorSign signer(params44);
    clwe::ColorSignVerify verifier(params44);

    // Test message
    std::vector<uint8_t> message = {'I', 'n', 't', 'e', 'g', 'r', 'a', 't', 'i', 'o', 'n', ' ', 't', 'e', 's', 't'};

    // Sign message
    clwe::ColorSignature signature = signer.sign_message(message, private_key, public_key);

    // Verify signature
    bool result = verifier.verify_signature(public_key, signature, message);

    EXPECT_TRUE(result);
}

TEST_F(IntegrationTest, FullSignVerifyCycle65) {
    clwe::ColorSignKeyGen keygen(params65);
    auto [public_key, private_key] = keygen.generate_keypair();

    clwe::ColorSign signer(params65);
    clwe::ColorSignVerify verifier(params65);

    std::vector<uint8_t> message = {'S', 'e', 'c', 'u', 'r', 'i', 't', 'y', ' ', 'l', 'e', 'v', 'e', 'l', ' ', '6', '5'};

    clwe::ColorSignature signature = signer.sign_message(message, private_key, public_key);
    bool result = verifier.verify_signature(public_key, signature, message);

    EXPECT_TRUE(result);
}

TEST_F(IntegrationTest, FullSignVerifyCycle87) {
    clwe::ColorSignKeyGen keygen(params87);
    auto [public_key, private_key] = keygen.generate_keypair();

    clwe::ColorSign signer(params87);
    clwe::ColorSignVerify verifier(params87);

    std::vector<uint8_t> message = {'H', 'i', 'g', 'h', 'e', 's', 't', ' ', 's', 'e', 'c', 'u', 'r', 'i', 't', 'y'};

    clwe::ColorSignature signature = signer.sign_message(message, private_key, public_key);
    bool result = verifier.verify_signature(public_key, signature, message);

    EXPECT_TRUE(result);
}

TEST_F(IntegrationTest, KeySerializationIntegration) {
    clwe::ColorSignKeyGen keygen(params44);
    auto [original_public, original_private] = keygen.generate_keypair();

    // Serialize keys
    auto serialized_public = original_public.serialize();
    auto serialized_private = original_private.serialize();

    // Deserialize keys
    auto deserialized_public = clwe::ColorSignPublicKey::deserialize(serialized_public, params44);
    auto deserialized_private = clwe::ColorSignPrivateKey::deserialize(serialized_private, params44);

    // Create signer and verifier with deserialized keys
    clwe::ColorSign signer(params44);
    clwe::ColorSignVerify verifier(params44);

    std::vector<uint8_t> message = {'S', 'e', 'r', 'i', 'a', 'l', 'i', 'z', 'a', 't', 'i', 'o', 'n', ' ', 't', 'e', 's', 't'};

    // Sign with deserialized private key
    clwe::ColorSignature signature = signer.sign_message(message, deserialized_private, deserialized_public);

    // Verify with deserialized public key
    bool result = verifier.verify_signature(deserialized_public, signature, message);

    EXPECT_TRUE(result);
}

TEST_F(IntegrationTest, SignatureSerializationIntegration) {
    clwe::ColorSignKeyGen keygen(params44);
    auto [public_key, private_key] = keygen.generate_keypair();

    clwe::ColorSign signer(params44);
    clwe::ColorSignVerify verifier(params44);

    std::vector<uint8_t> message = {'S', 'i', 'g', 'n', 'a', 't', 'u', 'r', 'e', ' ', 's', 'e', 'r', 'i', 'a', 'l'};

    // Sign and serialize signature
    clwe::ColorSignature original_signature = signer.sign_message(message, private_key, public_key);
    auto serialized_signature = original_signature.serialize();

    // Deserialize signature
    auto deserialized_signature = clwe::ColorSignature::deserialize(serialized_signature, params44);

    // Verify deserialized signature
    bool result = verifier.verify_signature(public_key, deserialized_signature, message);

    EXPECT_TRUE(result);
}

TEST_F(IntegrationTest, MultipleMessagesSameKey) {
    clwe::ColorSignKeyGen keygen(params44);
    auto [public_key, private_key] = keygen.generate_keypair();

    clwe::ColorSign signer(params44);
    clwe::ColorSignVerify verifier(params44);

    std::vector<std::vector<uint8_t>> messages = {
        {'F', 'i', 'r', 's', 't', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e'},
        {'S', 'e', 'c', 'o', 'n', 'd', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e'},
        {'T', 'h', 'i', 'r', 'd', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e'},
        {'F', 'o', 'u', 'r', 't', 'h', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e'}
    };

    for (const auto& message : messages) {
        clwe::ColorSignature signature = signer.sign_message(message, private_key, public_key);
        bool result = verifier.verify_signature(public_key, signature, message);
        EXPECT_TRUE(result);
    }
}

TEST_F(IntegrationTest, DeterministicKeyGenerationIntegration) {
    std::array<uint8_t, 32> seed = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
                                   0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                   0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00,
                                   0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x9A};

    clwe::ColorSignKeyGen keygen(params44);
    auto [public_key, private_key] = keygen.generate_keypair_deterministic(seed);

    clwe::ColorSign signer(params44);
    clwe::ColorSignVerify verifier(params44);

    std::vector<uint8_t> message = {'D', 'e', 't', 'e', 'r', 'm', 'i', 'n', 'i', 's', 't', 'i', 'c', ' ', 't', 'e', 's', 't'};

    clwe::ColorSignature signature = signer.sign_message(message, private_key, public_key);
    bool result = verifier.verify_signature(public_key, signature, message);

    EXPECT_TRUE(result);
}

TEST_F(IntegrationTest, LargeMessageIntegration) {
    clwe::ColorSignKeyGen keygen(params44);
    auto [public_key, private_key] = keygen.generate_keypair();

    clwe::ColorSign signer(params44);
    clwe::ColorSignVerify verifier(params44);

    // Create a large message (10KB)
    std::vector<uint8_t> large_message(10240);
    for (size_t i = 0; i < large_message.size(); ++i) {
        large_message[i] = static_cast<uint8_t>(i % 256);
    }

    clwe::ColorSignature signature = signer.sign_message(large_message, private_key, public_key);
    bool result = verifier.verify_signature(public_key, signature, large_message);

    EXPECT_TRUE(result);
}

TEST_F(IntegrationTest, EmptyMessageAfterSetup) {
    // This tests that empty message validation happens correctly
    clwe::ColorSignKeyGen keygen(params44);
    auto [public_key, private_key] = keygen.generate_keypair();

    clwe::ColorSign signer(params44);
    clwe::ColorSignVerify verifier(params44);

    std::vector<uint8_t> empty_message;

    // Signing should fail
    EXPECT_THROW(signer.sign_message(empty_message, private_key, public_key), std::invalid_argument);

    // Verification should fail
    clwe::ColorSignature dummy_signature{{}, {}, {0}, params44};
    EXPECT_THROW(verifier.verify_signature(public_key, dummy_signature, empty_message), std::invalid_argument);
}

TEST_F(IntegrationTest, CrossSecurityLevelFailure) {
    // Generate keys with one security level
    clwe::ColorSignKeyGen keygen44(params44);
    auto [public_key44, private_key44] = keygen44.generate_keypair();

    // Try to use with different security level
    clwe::ColorSign signer65(params65);
    clwe::ColorSignVerify verifier65(params65);

    std::vector<uint8_t> message = {'C', 'r', 'o', 's', 's', ' ', 'l', 'e', 'v', 'e', 'l'};

    // This should fail because parameters don't match
    EXPECT_THROW(signer65.sign_message(message, private_key44, public_key44), std::invalid_argument);
}

TEST_F(IntegrationTest, KnownAnswerTestDeterministic) {
    // Test with fixed seed for reproducible results
    std::array<uint8_t, 32> seed = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                                   0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
                                   0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                   0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00};

    clwe::ColorSignKeyGen keygen(params44);
    auto [public_key, private_key] = keygen.generate_keypair_deterministic(seed);

    clwe::ColorSign signer(params44);
    clwe::ColorSignVerify verifier(params44);

    std::vector<uint8_t> message = {'K', 'n', 'o', 'w', 'n', ' ', 'a', 'n', 's', 'w', 'e', 'r', ' ', 't', 'e', 's', 't'};

    // Sign multiple times - should eventually succeed (due to rejection sampling)
    clwe::ColorSignature signature;
    bool signed_successfully = false;
    for (int attempts = 0; attempts < 100 && !signed_successfully; ++attempts) {
        try {
            signature = signer.sign_message(message, private_key, public_key);
            signed_successfully = true;
        } catch (...) {
            // Continue trying
        }
    }

    ASSERT_TRUE(signed_successfully) << "Failed to generate signature after 100 attempts";

    // Verify the signature
    bool result = verifier.verify_signature(public_key, signature, message);
    EXPECT_TRUE(result);
}


} // namespace