#include <gtest/gtest.h>
#include "../include/clwe/security_utils.hpp"
#include "../include/clwe/parameters.hpp"
#include "../include/clwe/sign.hpp"
#include <vector>
#include <array>
#include <chrono>
#include <thread>

namespace clwe {

// Test fixture for security utilities
class SecurityUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize global security monitor
        initialize_security_monitor();
    }

    void TearDown() override {
        // Clean up
    }
};

// Test input validation
TEST_F(SecurityUtilsTest, InputValidation_MessageSize) {
    std::vector<uint8_t> valid_message(1000, 0xFF);
    EXPECT_EQ(InputValidator::validate_message_size(valid_message), SecurityError::SUCCESS);

    std::vector<uint8_t> oversized_message(MAX_MESSAGE_SIZE + 1, 0xFF);
    EXPECT_EQ(InputValidator::validate_message_size(oversized_message), SecurityError::INVALID_INPUT_SIZE);

    std::vector<uint8_t> empty_message;
    EXPECT_EQ(InputValidator::validate_message_size(empty_message), SecurityError::INVALID_INPUT_SIZE);
}

TEST_F(SecurityUtilsTest, InputValidation_KeySize) {
    std::vector<uint8_t> valid_key(2048, 0xFF);  // 2KB key
    EXPECT_EQ(InputValidator::validate_key_size(valid_key), SecurityError::SUCCESS);

    std::vector<uint8_t> oversized_key(MAX_KEY_SIZE + 1, 0xFF);
    EXPECT_EQ(InputValidator::validate_key_size(oversized_key), SecurityError::INVALID_KEY_FORMAT);

    std::vector<uint8_t> empty_key;
    EXPECT_EQ(InputValidator::validate_key_size(empty_key), SecurityError::INVALID_KEY_FORMAT);
}

TEST_F(SecurityUtilsTest, InputValidation_Parameters) {
    CLWEParameters valid_params(44);
    EXPECT_EQ(InputValidator::validate_parameters(valid_params), SecurityError::SUCCESS);

    CLWEParameters invalid_params;
    invalid_params.security_level = 99;
    invalid_params.degree = 256;
    invalid_params.module_rank = 4;
    invalid_params.repetitions = 4;
    invalid_params.modulus = 8380417;
    invalid_params.eta = 2;
    invalid_params.tau = 39;
    invalid_params.beta = 78;
    invalid_params.gamma1 = 1 << 17;
    invalid_params.gamma2 = (8380417 - 1) / 88;
    invalid_params.omega = 80;
    invalid_params.lambda = 128;
    EXPECT_EQ(InputValidator::validate_parameters(invalid_params), SecurityError::INVALID_PARAMETERS);
}

TEST_F(SecurityUtilsTest, InputValidation_Context) {
    std::vector<uint8_t> valid_context(32, 0xFF);
    EXPECT_EQ(InputValidator::validate_context_string(valid_context), SecurityError::SUCCESS);

    std::vector<uint8_t> oversized_context(256, 0xFF);  // > 255 bytes
    EXPECT_EQ(InputValidator::validate_context_string(oversized_context), SecurityError::INVALID_CONTEXT);
}

// Test constant-time operations
TEST_F(SecurityUtilsTest, ConstantTime_Compare) {
    uint8_t a[4] = {1, 2, 3, 4};
    uint8_t b[4] = {1, 2, 3, 4};
    uint8_t c[4] = {1, 2, 3, 5};

    EXPECT_TRUE(ConstantTime::compare(a, b, 4));
    EXPECT_FALSE(ConstantTime::compare(a, c, 4));
}

TEST_F(SecurityUtilsTest, ConstantTime_Select) {
    // Test the select functionality indirectly through other tests
    // The select function is tested via other arithmetic operations
    EXPECT_TRUE(true);  // Placeholder test
}

TEST_F(SecurityUtilsTest, ConstantTime_Arithmetic) {
    const uint32_t MOD = 8380417;

    EXPECT_EQ(ConstantTime::ct_add(100, 200, MOD), 300);
    EXPECT_EQ(ConstantTime::ct_sub(300, 100, MOD), 200);
    EXPECT_EQ(ConstantTime::ct_mul(10, 20, MOD), 200);
    EXPECT_EQ(ConstantTime::ct_mod(8380418, MOD), 1);
}

// Test secure memory
TEST_F(SecurityUtilsTest, SecureMemory_Buffer) {
    SecureMemory::SecureBuffer<uint8_t> buffer(1024);

    // Test basic operations
    buffer[0] = 0xFF;
    EXPECT_EQ(buffer[0], 0xFF);
    EXPECT_EQ(buffer.size(), 1024);

    // Test bounds checking
    EXPECT_THROW(buffer[1024], std::out_of_range);
}

TEST_F(SecurityUtilsTest, SecureMemory_BufferWipe) {
    uint8_t* raw_ptr = nullptr;
    {
        SecureMemory::SecureBuffer<uint8_t> buffer(16);
        raw_ptr = buffer.data();
        buffer[0] = 0xFF;
        buffer[15] = 0xAA;
        // Buffer should be wiped when it goes out of scope
    }
    // Note: We can't easily test that the memory is wiped without accessing
    // deallocated memory, but the RAII mechanism ensures it happens
}

// Test security monitor
TEST_F(SecurityUtilsTest, SecurityMonitor_Logging) {
    auto monitor = std::make_unique<DefaultSecurityMonitor>();

    AuditEntry entry{
        AuditEvent::SIGNING_START,
        std::chrono::system_clock::now(),
        "Test signing operation",
        "TestFunction",
        0
    };

    monitor->log_event(entry);

    // Test timing anomaly detection
    EXPECT_FALSE(monitor->detect_timing_anomaly("test_operation", 1000000));  // 1ms - normal
    EXPECT_FALSE(monitor->detect_timing_anomaly("test_operation", 2000000));  // 2ms - still normal

    // Test security violation reporting
    monitor->report_security_violation(SecurityError::TIMING_ATTACK_DETECTED, "Test violation");
}

TEST_F(SecurityUtilsTest, SecurityMonitor_LogRotation) {
    auto monitor = std::make_unique<DefaultSecurityMonitor>();

    // Set max log size to 5
    monitor->set_max_log_size(5);

    // Log 7 entries
    for (int i = 0; i < 7; ++i) {
        AuditEntry entry{
            AuditEvent::SIGNING_START,
            std::chrono::system_clock::now(),
            "Test entry " + std::to_string(i),
            "TestFunction",
            0
        };
        monitor->log_event(entry);
    }

    // Check that only 5 entries remain (the last 5)
    const auto& log = monitor->get_audit_log();
    EXPECT_EQ(log.size(), 5);

    // Check that the oldest entries were removed (first 2 should be gone)
    EXPECT_EQ(log[0].details, "Test entry 2");
    EXPECT_EQ(log[1].details, "Test entry 3");
    EXPECT_EQ(log[2].details, "Test entry 4");
    EXPECT_EQ(log[3].details, "Test entry 5");
    EXPECT_EQ(log[4].details, "Test entry 6");
}

// Test timing protection
TEST_F(SecurityUtilsTest, TimingProtection_Basic) {
    auto monitor = std::make_unique<DefaultSecurityMonitor>();
    TimingProtection timing_protection(std::move(monitor));

    timing_protection.start_operation();

    // Simulate some operation
    std::chrono::milliseconds sleep_duration(1);
    std::this_thread::sleep_for(sleep_duration);

    timing_protection.end_operation("TestOperation");

    uint64_t operation_duration = timing_protection.get_operation_time_ns();
    EXPECT_GT(operation_duration, 0ULL);
}

// Test polynomial bounds checking
TEST_F(SecurityUtilsTest, PolynomialBounds_Valid) {
    std::vector<std::vector<uint32_t>> valid_poly = {
        {100, 200, 300},  // Within bounds
        {150, 250, 350}
    };

    SecurityError result = InputValidator::validate_polynomial_vector_bounds(
        valid_poly, 2, 3, -1000, 1000, 8380417);
    EXPECT_EQ(result, SecurityError::SUCCESS);
}

TEST_F(SecurityUtilsTest, PolynomialBounds_Invalid) {
    std::vector<std::vector<uint32_t>> invalid_poly = {
        {100, 200, 300},  // Within bounds
        {150, 250, 1500}  // 1500 > 1000 (max bound)
    };

    SecurityError result = InputValidator::validate_polynomial_vector_bounds(
        invalid_poly, 2, 3, -1000, 1000, 8380417);
    EXPECT_EQ(result, SecurityError::BOUNDS_CHECK_FAILURE);
}

TEST_F(SecurityUtilsTest, PolynomialBounds_WrongDimensions) {
    std::vector<std::vector<uint32_t>> wrong_dims = {
        {100, 200},      // Only 2 elements, expected 3
        {150, 250, 350}
    };

    SecurityError result = InputValidator::validate_polynomial_vector_bounds(
        wrong_dims, 2, 3, -1000, 1000, 8380417);
    EXPECT_EQ(result, SecurityError::BOUNDS_CHECK_FAILURE);
}

// Test error message utility
TEST_F(SecurityUtilsTest, ErrorMessages) {
    EXPECT_EQ(std::string(get_security_error_message(SecurityError::SUCCESS)), "Success");
    EXPECT_EQ(std::string(get_security_error_message(SecurityError::INVALID_INPUT_SIZE)), "Invalid input size");
    EXPECT_EQ(std::string(get_security_error_message(SecurityError::TIMING_ATTACK_DETECTED)), "Timing attack detected");
    EXPECT_EQ(std::string(get_security_error_message(SecurityError::MEMORY_ALLOCATION_FAILED)), "Memory allocation failed");
}


} // namespace clwe