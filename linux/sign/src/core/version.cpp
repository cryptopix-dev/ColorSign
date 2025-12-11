#include "../include/clwe/version.hpp"
#include <sstream>

namespace clwe {

// Get version string
std::string get_version_string() {
    std::stringstream ss;
    ss << COLORSIGN_VERSION_MAJOR << "."
       << COLORSIGN_VERSION_MINOR << "."
       << COLORSIGN_VERSION_PATCH;
    if (COLORSIGN_VERSION_BUILD > 0) {
        ss << "-build." << COLORSIGN_VERSION_BUILD;
    }
    return ss.str();
}

// Get build information
std::string get_build_info() {
    std::stringstream ss;
    ss << "ColorSign " << get_version_string() << "\n";
    ss << "Built on: " << COLORSIGN_BUILD_DATE << " " << COLORSIGN_BUILD_TIME << "\n";
    ss << "Compiler: " << COLORSIGN_COMPILER << "\n";
    ss << "Security Standards: FIPS 204 ML-DSA\n";
    ss << "Supported Platforms: Linux\n";
    return ss.str();
}

// Get security level name
std::string get_security_level_name(int level) {
    switch (level) {
        case 44:
            return "ML-DSA-44";
        case 65:
            return "ML-DSA-65";
        case 87:
            return "ML-DSA-87";
        default:
            return "Unknown";
    }
}

// Check if security level is supported
bool is_supported_security_level(int level) {
    return level == 44 || level == 65 || level == 87;
}

} // namespace clwe