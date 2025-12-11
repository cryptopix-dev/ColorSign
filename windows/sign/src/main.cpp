#include "include/clwe/keygen.hpp"
#include "include/clwe/parameters.hpp"
#include "include/clwe/sign.hpp"
#include "include/clwe/verify.hpp"
#include <iostream>
#include <vector>

int main() {
    try {
        clwe::CLWEParameters params(44);
        clwe::ColorSignKeyGen keygen(params);

        std::cout << "Generating ColorSign keypair..." << std::endl;
        auto [public_key, private_key] = keygen.generate_keypair();

        std::cout << "Key generation successful!" << std::endl;
        std::cout << "Public key seed_rho size: " << public_key.seed_rho.size() << " bytes" << std::endl;
        std::cout << "Public key data size: " << public_key.public_data.size() << " bytes" << std::endl;
        std::cout << "Private key data size: " << private_key.secret_data.size() << " bytes" << std::endl;

        clwe::ColorSign signer(params);
        std::vector<uint8_t> message = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};

        std::cout << "Signing message..." << std::endl;
        auto signature = signer.sign_message(message, private_key, public_key);

        std::cout << "Signing successful!" << std::endl;
        std::cout << "Signature z_data size: " << signature.z_data.size() << " bytes" << std::endl;
        std::cout << "Signature c_data size: " << signature.c_data.size() << " bytes" << std::endl;

        auto serialized_sig = signature.serialize();

        std::cout << "Signature serialization successful!" << std::endl;
        std::cout << "Serialized signature size: " << serialized_sig.size() << " bytes" << std::endl;

        auto deserialized_sig = clwe::ColorSignature::deserialize(serialized_sig, params);

        std::cout << "Signature deserialization successful!" << std::endl;

        clwe::ColorSignVerify verifier(params);

        std::cout << "Verifying signature..." << std::endl;
        bool is_valid = verifier.verify_signature(public_key, signature, message);

        if (is_valid) {
            std::cout << "Signature verification successful!" << std::endl;
        } else {
            std::cout << "Signature verification failed!" << std::endl;
            return 1;
        }

        std::vector<uint8_t> wrong_message = {'W', 'r', 'o', 'n', 'g'};
        bool is_invalid = verifier.verify_signature(public_key, signature, wrong_message);

        if (!is_invalid) {
            std::cout << "Wrong message correctly rejected!" << std::endl;
        } else {
            std::cout << "Error: Wrong message was accepted!" << std::endl;
            return 1;
        }

        std::cout << "All tests passed!" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}