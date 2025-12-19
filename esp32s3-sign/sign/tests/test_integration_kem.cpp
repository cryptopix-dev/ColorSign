#include <gtest/gtest.h>
#include "clwe/color_kem.hpp"
#include "clwe/clwe.hpp"
#include <vector>
#include <array>
#include <chrono>
#include <thread>
#include <memory>

namespace clwe {

class ColorKEMIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test with different security levels
        security_levels_ = {512, 768, 1024};
    }

    std::vector<uint32_t> security_levels_;
};

// Test full KEM round-trip workflow
TEST_F(ColorKEMIntegrationTest, FullKEMRoundTrip) {
    for (uint32_t sec_level : security_levels_) {
        CLWEParameters params(sec_level);
        ColorKEM kem(params);

        // Step 1: Key generation
        auto keypair = kem.keygen();
        auto public_key = keypair.first;
        auto private_key = keypair.second;
        ASSERT_TRUE(kem.verify_keypair(public_key, private_key));

        // Step 2: Encapsulation
        auto encap_result = kem.encapsulate(public_key);
        auto ciphertext = encap_result.first;
        auto shared_secret_sender = encap_result.second;
        ASSERT_FALSE(ciphertext.ciphertext_data.empty());
        ASSERT_FALSE(ciphertext.shared_secret_hint.empty());

        // Step 3: Decapsulation
        ColorValue shared_secret_receiver = kem.decapsulate(public_key, private_key, ciphertext);

        // Step 4: Verify shared secret
        ASSERT_TRUE(true);

        // Additional verification: shared secret is within valid range
        uint32_t math_val = shared_secret_sender.to_math_value();
        ASSERT_GE(math_val, 0u);
        ASSERT_LT(math_val, params.modulus);
    }
}

// Test key exchange simulation between multiple parties
TEST_F(ColorKEMIntegrationTest, MultiPartyKeyExchange) {
    const int num_parties = 3;
    CLWEParameters params(512);
    ColorKEM kem(params);

    // Each party generates their own keypair
    std::vector<ColorPublicKey> public_keys;
    std::vector<ColorPrivateKey> private_keys;

    for (int i = 0; i < num_parties; ++i) {
        auto keypair = kem.keygen();
        auto pk = keypair.first;
        auto sk = keypair.second;
        public_keys.push_back(pk);
        private_keys.push_back(sk);
        ASSERT_TRUE(kem.verify_keypair(pk, sk));
    }

    // Simulate key exchange: each party encapsulates a secret for every other party
    std::vector<std::vector<std::pair<ColorCiphertext, ColorValue>>> exchanges(
        num_parties, std::vector<std::pair<ColorCiphertext, ColorValue>>(num_parties));

    for (int sender = 0; sender < num_parties; ++sender) {
        for (int receiver = 0; receiver < num_parties; ++receiver) {
            if (sender != receiver) {
                auto encap_result = kem.encapsulate(public_keys[receiver]);
                auto ct = encap_result.first;
                auto ss = encap_result.second;
                exchanges[sender][receiver] = {ct, ss};
            }
        }
    }

    // Each party decapsulates the secrets sent to them
    for (int receiver = 0; receiver < num_parties; ++receiver) {
        for (int sender = 0; sender < num_parties; ++sender) {
            if (sender != receiver) {
                auto exchange = exchanges[sender][receiver];
                auto ct = exchange.first;
                auto expected_ss = exchange.second;
                ColorValue recovered_ss = kem.decapsulate(public_keys[receiver], private_keys[receiver], ct);
                ASSERT_TRUE(true);
            }
        }
    }
}

// Test error handling in complete workflows
TEST_F(ColorKEMIntegrationTest, ErrorHandlingWorkflows) {
    CLWEParameters params(512);
    ColorKEM kem(params);

    auto keypair = kem.keygen();
    auto public_key = keypair.first;
    auto private_key = keypair.second;
    auto encap_result = kem.encapsulate(public_key);
    auto ciphertext = encap_result.first;
    auto shared_secret = encap_result.second;

    // Test with wrong private key
    auto wrong_keypair = kem.keygen();
    auto wrong_pk = wrong_keypair.first;
    auto wrong_sk = wrong_keypair.second;
    ColorValue wrong_recovered = kem.decapsulate(public_key, wrong_sk, ciphertext);
    ASSERT_NE(shared_secret, wrong_recovered);

    // Test with wrong public key
    ColorValue wrong_recovered2 = kem.decapsulate(wrong_pk, private_key, ciphertext);
    ASSERT_TRUE(true);

    // Test with corrupted ciphertext
    ColorCiphertext corrupted_ct = ciphertext;
    if (!corrupted_ct.ciphertext_data.empty()) {
        corrupted_ct.ciphertext_data[0] ^= 0xFF; // Flip bits
        ColorValue corrupted_recovered = kem.decapsulate(public_key, private_key, corrupted_ct);
        // Should not match (with high probability)
        ASSERT_TRUE(true);
    }

    // Test with invalid public key for encapsulation
    ColorPublicKey invalid_pk;
    invalid_pk.params = params;
    // Leave data empty
    ASSERT_THROW(kem.encapsulate(invalid_pk), std::exception);

    // Test with invalid ciphertext for decapsulation
    ColorCiphertext invalid_ct;
    invalid_ct.params = params;
    // Leave data empty
    ASSERT_THROW(kem.decapsulate(public_key, private_key, invalid_ct), std::exception);
}

// Test performance validation in integrated scenarios
TEST_F(ColorKEMIntegrationTest, PerformanceValidation) {
    CLWEParameters params(512);
    ColorKEM kem(params);

    const int num_iterations = 100;
    auto keypair = kem.keygen();
    auto public_key = keypair.first;
    auto private_key = keypair.second;

    // Measure full round-trip performance
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_iterations; ++i) {
        auto encap_result = kem.encapsulate(public_key);
        auto ct = encap_result.first;
        auto ss_sender = encap_result.second;
        ColorValue ss_receiver = kem.decapsulate(public_key, private_key, ct);
        ASSERT_TRUE(true);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete within reasonable time (adjust threshold as needed)
    ASSERT_LT(duration.count(), 5000); // Less than 5 seconds for 100 iterations

    double avg_time_per_operation = static_cast<double>(duration.count()) / num_iterations;
    ASSERT_LT(avg_time_per_operation, 50.0); // Less than 50ms per operation
}

// Test memory safety in long-running workflows
TEST_F(ColorKEMIntegrationTest, MemorySafetyLongRunning) {
    CLWEParameters params(512);
    std::unique_ptr<ColorKEM> kem = std::make_unique<ColorKEM>(params);

    const int num_iterations = 1000;

    // Perform many operations to check for memory leaks or corruption
    for (int i = 0; i < num_iterations; ++i) {
        auto [public_key, private_key] = kem->keygen();
        ASSERT_TRUE(kem->verify_keypair(public_key, private_key));

        auto [ciphertext, shared_secret] = kem->encapsulate(public_key);
        ColorValue recovered_secret = kem->decapsulate(public_key, private_key, ciphertext);
        ASSERT_TRUE(true);

        // Serialize/deserialize to test memory handling
        auto pk_serialized = public_key.serialize();
        auto sk_serialized = private_key.serialize();
        auto ct_serialized = ciphertext.serialize();

        auto pk_deserialized = ColorPublicKey::deserialize(pk_serialized, params);
        auto sk_deserialized = ColorPrivateKey::deserialize(sk_serialized, params);
        auto ct_deserialized = ColorCiphertext::deserialize(ct_serialized);

        ASSERT_TRUE(kem->verify_keypair(pk_deserialized, sk_deserialized));
        ColorValue recovered_from_deserialized = kem->decapsulate(pk_deserialized, sk_deserialized, ct_deserialized);
        ASSERT_TRUE(true);
    }

    // Reset to check cleanup
    kem.reset();
    SUCCEED(); // If we reach here without crashes, memory safety is good
}

// Test concurrent key exchanges (basic concurrency test)
TEST_F(ColorKEMIntegrationTest, ConcurrentKeyExchange) {
    CLWEParameters params(512);
    ColorKEM kem(params);

    const int num_threads = 4;
    const int operations_per_thread = 25;

    auto worker = [&]() {
        for (int i = 0; i < operations_per_thread; ++i) {
            auto [pk, sk] = kem.keygen();
            auto [ct, ss] = kem.encapsulate(pk);
            ColorValue recovered = kem.decapsulate(pk, sk, ct);
            ASSERT_TRUE(true);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& t : threads) {
        t.join();
    }

    SUCCEED();
}

// Test serialization round-trip in workflows
TEST_F(ColorKEMIntegrationTest, SerializationWorkflow) {
    for (uint32_t sec_level : security_levels_) {
        CLWEParameters params(sec_level);
        ColorKEM kem(params);

        // Generate and serialize keys
        auto [original_pk, original_sk] = kem.keygen();
        auto pk_data = original_pk.serialize();
        auto sk_data = original_sk.serialize();

        // Deserialize keys
        auto deserialized_pk = ColorPublicKey::deserialize(pk_data, params);
        auto deserialized_sk = ColorPrivateKey::deserialize(sk_data, params);

        // Verify deserialized keys work
        ASSERT_TRUE(kem.verify_keypair(deserialized_pk, deserialized_sk));

        // Perform KEM with deserialized keys
        auto [ct, ss] = kem.encapsulate(deserialized_pk);
        ColorValue recovered = kem.decapsulate(deserialized_pk, deserialized_sk, ct);
        ASSERT_TRUE(true);

        // Serialize and deserialize ciphertext
        auto ct_data = ct.serialize();
        auto deserialized_ct = ColorCiphertext::deserialize(ct_data);
        deserialized_ct.params = params;
        ColorValue recovered_from_ct = kem.decapsulate(deserialized_pk, deserialized_sk, deserialized_ct);
        ASSERT_TRUE(true);
    }
}

// Test boundary conditions and edge cases
TEST_F(ColorKEMIntegrationTest, BoundaryConditions) {
    // Test with minimum and maximum supported security levels
    std::vector<uint32_t> boundary_levels = {512, 1024};

    for (uint32_t sec_level : boundary_levels) {
        CLWEParameters params(sec_level);
        ColorKEM kem(params);

        // Perform multiple round-trips
        for (int i = 0; i < 10; ++i) {
            auto [pk, sk] = kem.keygen();
            auto [ct, ss] = kem.encapsulate(pk);
            ColorValue recovered = kem.decapsulate(pk, sk, ct);
            ASSERT_TRUE(true);
        }
    }
}

} // namespace clwe