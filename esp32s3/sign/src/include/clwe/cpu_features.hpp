/**
 * @file cpu_features.hpp
 * @brief CPU feature detection for SIMD optimization
 *
 * This header provides runtime detection of CPU capabilities to enable
 * optimal SIMD acceleration for cryptographic operations. The ColorSign
 * library uses various SIMD instruction sets (AVX-512, AVX2, NEON, etc.)
 * to accelerate polynomial arithmetic and other compute-intensive operations.
 *
 * The feature detector automatically identifies the best available SIMD
 * instructions and allows the library to dispatch to optimized implementations
 * at runtime.
 *
 * @author ColorSign Development Team
 * @version 1.0.0
 * @date 2024
 */

#ifndef CPU_FEATURES_HPP
#define CPU_FEATURES_HPP

#include <cstdint>
#include <string>

// Include for SIMDSupport enum
#include "ntt_engine.hpp"

namespace clwe {

/** @brief Supported CPU architectures */
enum class CPUArchitecture {
    UNKNOWN,  /**< Unknown or unsupported architecture */
    X86_64,   /**< x86-64 (Intel/AMD) architecture */
    ARM64,    /**< ARM64 (AArch64) architecture */
    RISCV64,  /**< RISC-V 64-bit architecture */
    PPC64     /**< PowerPC 64-bit architecture */
};


/**
 * @brief CPU feature information structure
 *
 * Contains detailed information about detected CPU capabilities,
 * including architecture type and available SIMD instruction sets.
 * This information is used to select optimal implementations at runtime.
 */
struct CPUFeatures {
    CPUArchitecture architecture = CPUArchitecture::UNKNOWN;  /**< Detected CPU architecture */
    SIMDSupport max_simd_support = SIMDSupport::NONE;          /**< Highest SIMD support level */

    // x86-64 specific features
    bool has_avx2 = false;      /**< AVX2 instruction set support */
    bool has_avx512f = false;   /**< AVX-512 Foundation instructions */
    bool has_avx512dq = false;  /**< AVX-512 Doubleword/Quadword instructions */
    bool has_avx512bw = false;  /**< AVX-512 Byte/Word instructions */
    bool has_avx512vl = false;  /**< AVX-512 Vector Length extensions */

    // ARM-specific features
    bool has_neon = false;      /**< ARM NEON SIMD support */
    bool has_sve = false;       /**< ARM Scalable Vector Extension */

    // RISC-V specific features
    bool has_rvv = false;       /**< RISC-V Vector extension */
    uint32_t rvv_vlen = 0;      /**< RISC-V vector length in bits */

    // PowerPC specific features
    bool has_vsx = false;       /**< PowerPC VSX instructions */
    bool has_altivec = false;   /**< PowerPC AltiVec instructions */

    /**
     * @brief Convert CPU features to human-readable string
     *
     * @return std::string Description of detected CPU features
     */
    std::string to_string() const;
};

/**
 * @brief CPU feature detection utility class
 *
 * Provides static methods for detecting CPU capabilities at runtime.
 * This allows ColorSign to automatically select the most efficient
 * implementation based on available hardware features.
 *
 * The detector supports multiple architectures and SIMD instruction sets,
 * providing a unified interface for feature detection across platforms.
 */
class CPUFeatureDetector {
public:
    /**
     * @brief Detect all available CPU features
     *
     * Performs comprehensive CPU feature detection including:
     * - CPU architecture identification
     * - SIMD instruction set availability
     * - Specific instruction support (AVX-512 variants, NEON, etc.)
     *
     * @return CPUFeatures Structure containing all detected features
     *
     * @note This method is thread-safe and can be called multiple times
     */
    static CPUFeatures detect();

private:
    /** @brief Detect x86-64 specific features using CPUID */
    static CPUFeatures detect_x86();

    /** @brief Detect ARM64 specific features */
    static CPUFeatures detect_arm();

    /** @brief Detect RISC-V specific features */
    static CPUFeatures detect_riscv();

    /** @brief Detect PowerPC specific features */
    static CPUFeatures detect_ppc();

    /** @brief Detect the CPU architecture */
    static CPUArchitecture detect_architecture();

    // x86-specific helper functions
    /** @brief Check if CPUID instruction is available */
    static bool has_cpuid();

    /**
     * @brief Execute CPUID instruction
     * @param leaf CPUID leaf
     * @param subleaf CPUID subleaf
     * @param regs Output array for EAX, EBX, ECX, EDX registers
     */
    static void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t* regs);

    /**
     * @brief Execute XGETBV instruction for extended state detection
     * @param xcr XCR register number
     * @return uint64_t XCR register value
     */
    static uint64_t xgetbv(uint32_t xcr);
};

} // namespace clwe

#endif // CPU_FEATURES_HPP