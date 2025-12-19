#ifndef CLWE_VERSION_HPP
#define CLWE_VERSION_HPP

#include <string>

namespace clwe {

// Version information
constexpr int COLORSIGN_VERSION_MAJOR = 1;
constexpr int COLORSIGN_VERSION_MINOR = 0;
constexpr int COLORSIGN_VERSION_PATCH = 0;
constexpr int COLORSIGN_VERSION_BUILD = 1;

// Version string
constexpr const char* COLORSIGN_VERSION_STRING = "1.0.0";

// Build metadata
constexpr const char* COLORSIGN_BUILD_DATE = __DATE__;
constexpr const char* COLORSIGN_BUILD_TIME = __TIME__;
constexpr const char* COLORSIGN_COMPILER = "AppleClang";

// Security level information
enum class SecurityLevel {
    ML_DSA_44 = 44,
    ML_DSA_65 = 65,
    ML_DSA_87 = 87
};

// Get version information
std::string get_version_string();
std::string get_build_info();
std::string get_security_level_name(int level);
bool is_supported_security_level(int level);

} // namespace clwe

#endif // CLWE_VERSION_HPP