#include "../include/clwe/color_integration.hpp"
#include <stdexcept>

namespace clwe {

std::vector<uint8_t> encode_polynomial_as_colors(const std::vector<uint32_t>& poly, uint32_t modulus) {
    std::vector<uint8_t> color_data;

    for (uint32_t coeff : poly) {
        // Reduce modulo modulus first (skip if modulus is 0)
        if (modulus != 0) {
            coeff %= modulus;
        }
        // Pack into 4 bytes (RGBA)
        color_data.push_back((coeff >> 24) & 0xFF);
        color_data.push_back((coeff >> 16) & 0xFF);
        color_data.push_back((coeff >> 8) & 0xFF);
        color_data.push_back(coeff & 0xFF);
    }

    return color_data;
}

std::vector<uint32_t> decode_colors_to_polynomial(const std::vector<uint8_t>& color_data, uint32_t modulus) {
    if (color_data.size() % 4 != 0) {
        throw std::invalid_argument("Color data size must be multiple of 4");
    }

    std::vector<uint32_t> poly;
    poly.reserve(color_data.size() / 4);

    for (size_t i = 0; i < color_data.size(); i += 4) {
        uint32_t coeff = ((uint32_t)color_data[i] << 24) |
                         ((uint32_t)color_data[i + 1] << 16) |
                         ((uint32_t)color_data[i + 2] << 8) |
                         (uint32_t)color_data[i + 3];
        // Apply modulus only if it's not zero
        poly.push_back(modulus != 0 ? coeff % modulus : coeff);
    }

    return poly;
}

std::vector<uint8_t> encode_polynomial_vector_as_colors(const std::vector<std::vector<uint32_t>>& poly_vector, uint32_t modulus) {
    std::vector<uint8_t> color_data;

    for (const auto& poly : poly_vector) {
        auto poly_colors = encode_polynomial_as_colors(poly, modulus);
        color_data.insert(color_data.end(), poly_colors.begin(), poly_colors.end());
    }

    return color_data;
}

std::vector<std::vector<uint32_t>> decode_colors_to_polynomial_vector(const std::vector<uint8_t>& color_data, uint32_t k, uint32_t n, uint32_t modulus) {
    if (color_data.size() != k * n * 4) {
        throw std::invalid_argument("Color data size does not match expected dimensions");
    }

    std::vector<std::vector<uint32_t>> poly_vector(k, std::vector<uint32_t>(n));

    size_t idx = 0;
    for (uint32_t i = 0; i < k; ++i) {
        for (uint32_t j = 0; j < n; ++j) {
            uint32_t coeff = ((uint32_t)color_data[idx] << 24) |
                             ((uint32_t)color_data[idx + 1] << 16) |
                             ((uint32_t)color_data[idx + 2] << 8) |
                             (uint32_t)color_data[idx + 3];
            // Apply modulus only if it's not zero
            poly_vector[i][j] = modulus != 0 ? coeff % modulus : coeff;
            idx += 4;
        }
    }

    return poly_vector;
}

} // namespace clwe