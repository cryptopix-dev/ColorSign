#include "src/include/clwe/keygen.hpp"
#include "src/include/clwe/parameters.hpp"
#include "src/include/clwe/sign.hpp"
#include "src/include/clwe/verify.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>

int main() {
    try {
        std::cout << "ColorSign Benchmark - Timing Test" << std::endl;
        std::cout << "=================================" << std::endl;

        // Test with different parameter sets
        std::vector<int> parameter_sets = {44, 65, 87};  // ML-DSA-44, ML-DSA-65, ML-DSA-87

        for (int param_set : parameter_sets) {
            std::cout << "\nTesting ML-DSA-" << param_set << " parameters:" << std::endl;

            clwe::CLWEParameters params(param_set);
            clwe::ColorSignKeyGen keygen(params);

            // Key generation timing
            auto start = std::chrono::high_resolution_clock::now();
            auto [public_key, private_key] = keygen.generate_keypair();
            auto end = std::chrono::high_resolution_clock::now();
            auto keygen_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            std::cout << "  Key generation: " << keygen_time_us << " μs" << std::endl;

            // Create a test message
            std::vector<uint8_t> message(1024); // 1KB message
            std::fill(message.begin(), message.end(), 0xAA);

            // Signing timing
            clwe::ColorSign signer(params);
            start = std::chrono::high_resolution_clock::now();
            auto signature = signer.sign_message(message, private_key, public_key);
            end = std::chrono::high_resolution_clock::now();
            auto sign_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            std::cout << "  Signing: " << sign_time_us << " μs" << std::endl;

            // Verification timing
            clwe::ColorSignVerify verifier(params);
            start = std::chrono::high_resolution_clock::now();
            bool is_valid = verifier.verify_signature(public_key, signature, message);
            end = std::chrono::high_resolution_clock::now();
            auto verify_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            std::cout << "  Verification: " << verify_time_us << " μs" << std::endl;
            std::cout << "  Verification result: " << (is_valid ? "SUCCESS" : "FAILED") << std::endl;

            // Signature size
            auto serialized_sig = signature.serialize();
            std::cout << "  Signature size: " << serialized_sig.size() << " bytes" << std::endl;
        }

        std::cout << "\nBenchmark completed successfully!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Benchmark error: " << e.what() << std::endl;
        return 1;
    }
}