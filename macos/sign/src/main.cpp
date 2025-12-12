#include "include/clwe/keygen.hpp"
#include "include/clwe/parameters.hpp"
#include "include/clwe/sign.hpp"
#include "include/clwe/verify.hpp"
#include "include/clwe/color_integration.hpp"
#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <webp/encode.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

std::string base64_encode(const uint8_t* data, size_t len) {
    BIO *bio = BIO_new(BIO_s_mem());
    BIO *b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data, len);
    BIO_flush(bio);
    BUF_MEM *bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    return result;
}

std::string encode_webp_base64(const std::vector<uint8_t>& rgb_data) {
    size_t num_pixels = rgb_data.size() / 3;
    size_t width = static_cast<size_t>(std::ceil(std::sqrt(static_cast<double>(num_pixels))));
    size_t height = (num_pixels + width - 1) / width;
    std::vector<uint8_t> image(width * height * 3, 0);
    for (size_t i = 0; i < num_pixels * 3; ++i) {
        image[i] = rgb_data[i];
    }
    uint8_t* webp_data = nullptr;
    size_t webp_size = WebPEncodeLosslessRGB(image.data(), width, height, static_cast<int>(width * 3), &webp_data);
    if (webp_size == 0) {
        return "";
    }
    std::string b64 = base64_encode(webp_data, webp_size);
    WebPFree(webp_data);
    return b64;
}

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

        std::cout << "Public key seed_rho WebP (base64): " << encode_webp_base64(std::vector<uint8_t>(public_key.seed_rho.begin(), public_key.seed_rho.end())) << std::endl;

        // Convert compressed data to RGB for WebP display
        std::vector<uint8_t> public_display_data;
        if (public_key.use_compression) {
            auto t = clwe::unpack_polynomial_vector_ml_dsa(public_key.public_data, params.module_rank, params.degree, params.modulus, 12);
            public_display_data = clwe::encode_polynomial_vector_as_colors(t, params.modulus);
        } else {
            public_display_data = public_key.public_data;
        }
        std::cout << "Public key data WebP (base64): " << encode_webp_base64(public_display_data) << std::endl;

        std::vector<uint8_t> private_display_data;
        if (private_key.use_compression) {
            auto all_secret = clwe::unpack_polynomial_vector_ml_dsa(private_key.secret_data, 2 * params.module_rank, params.degree, params.modulus, 4);
            std::vector<std::vector<uint32_t>> s1(all_secret.begin(), all_secret.begin() + params.module_rank);
            std::vector<std::vector<uint32_t>> s2(all_secret.begin() + params.module_rank, all_secret.end());
            auto s1_colors = clwe::encode_polynomial_vector_as_colors(s1, params.modulus);
            auto s2_colors = clwe::encode_polynomial_vector_as_colors(s2, params.modulus);
            private_display_data = s1_colors;
            private_display_data.insert(private_display_data.end(), s2_colors.begin(), s2_colors.end());
        } else {
            private_display_data = private_key.secret_data;
        }
        std::cout << "Private key data WebP (base64): " << encode_webp_base64(private_display_data) << std::endl;

        clwe::ColorSign signer(params);
        std::vector<uint8_t> message = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};

        std::cout << "Signing message..." << std::endl;
        auto signature = signer.sign_message(message, private_key, public_key);

        std::cout << "Signing successful!" << std::endl;
        std::cout << "Signature z_data size: " << signature.z_data.size() << " bytes" << std::endl;
        std::cout << "Signature c_data size: " << signature.c_data.size() << " bytes" << std::endl;

        // Verify with original signature before serialization
        clwe::ColorSignVerify verifier(params);
        std::cout << "Verifying signature with original..." << std::endl;
        bool is_valid_orig = verifier.verify_signature(public_key, signature, message);
        std::cout << "Verification with original signature: " << (is_valid_orig ? "successful" : "failed") << std::endl;

        auto serialized_sig = signature.serialize();

        std::cout << "Signature serialization successful!" << std::endl;
        std::cout << "Serialized signature size: " << serialized_sig.size() << " bytes" << std::endl;

        auto deserialized_sig = clwe::ColorSignature::deserialize(serialized_sig, params);

        std::cout << "Signature deserialization successful!" << std::endl;

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