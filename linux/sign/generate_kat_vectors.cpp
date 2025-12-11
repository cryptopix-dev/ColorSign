#include "src/include/clwe/kat.hpp"
#include "src/include/clwe/parameters.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: " << argv[0] << " <parameter_set> <output_file>" << std::endl;
            std::cerr << "  parameter_set: 44, 65, or 87" << std::endl;
            return 1;
        }

        int param_set = std::stoi(argv[1]);
        std::string output_file = argv[2];

        std::cout << "Generating KAT vectors for ML-DSA-" << param_set << "..." << std::endl;

        clwe::CLWEParameters params(param_set);
        clwe::ColorSignKAT kat_test(params);

        // Run KAT tests
        bool success = kat_test.run_all_kats();
        if (!success) {
            throw std::runtime_error("KAT tests failed for ML-DSA-" + std::to_string(param_set));
        }

        // Get test vectors
        auto kat_vectors = clwe::ColorSignKAT::get_test_vectors(param_set);

        // Write to file
        std::ofstream out_file(output_file, std::ios::binary);
        if (!out_file) {
            throw std::runtime_error("Failed to open output file: " + output_file);
        }

        // Write KAT vectors in binary format
        for (const auto& vec : kat_vectors) {
            // Write security level
            out_file.write(reinterpret_cast<const char*>(&vec.security_level), sizeof(vec.security_level));

            // Write seed
            out_file.write(reinterpret_cast<const char*>(vec.seed.data()), vec.seed.size());

            // Write message size and data
            uint32_t msg_size = static_cast<uint32_t>(vec.message.size());
            out_file.write(reinterpret_cast<const char*>(&msg_size), sizeof(msg_size));
            out_file.write(reinterpret_cast<const char*>(vec.message.data()), vec.message.size());

            // Write expected public key size and data
            uint32_t pk_size = static_cast<uint32_t>(vec.expected_pk.size());
            out_file.write(reinterpret_cast<const char*>(&pk_size), sizeof(pk_size));
            out_file.write(reinterpret_cast<const char*>(vec.expected_pk.data()), vec.expected_pk.size());

            // Write expected private key size and data
            uint32_t sk_size = static_cast<uint32_t>(vec.expected_sk.size());
            out_file.write(reinterpret_cast<const char*>(&sk_size), sizeof(sk_size));
            out_file.write(reinterpret_cast<const char*>(vec.expected_sk.data()), vec.expected_sk.size());

            // Write expected signature size and data
            uint32_t sig_size = static_cast<uint32_t>(vec.expected_sig.size());
            out_file.write(reinterpret_cast<const char*>(&sig_size), sizeof(sig_size));
            out_file.write(reinterpret_cast<const char*>(vec.expected_sig.data()), vec.expected_sig.size());
        }

        std::cout << "Successfully generated " << kat_vectors.size() << " KAT vectors" << std::endl;
        std::cout << "Total size: " << out_file.tellp() << " bytes" << std::endl;
        std::cout << "Saved to: " << output_file << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "KAT generation error: " << e.what() << std::endl;
        return 1;
    }
}