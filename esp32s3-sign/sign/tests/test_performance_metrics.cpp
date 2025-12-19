#include <gtest/gtest.h>
#include "performance_metrics.hpp"
#include <thread>
#include <chrono>
#include <cmath>

using namespace clwe;

class PerformanceMetricsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test memory usage measurement
TEST_F(PerformanceMetricsTest, GetMemoryUsage) {
    MemoryStats mem = PerformanceMetrics::get_memory_usage();
    EXPECT_GE(mem.current_memory, 0);
    EXPECT_GE(mem.peak_memory, 0);
    EXPECT_GE(mem.average_memory, 0);
}

// Test timing functionality
TEST_F(PerformanceMetricsTest, TimeOperation) {
    auto operation = []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    };

    TimingStats timing = PerformanceMetrics::time_operation(operation, 5);

    EXPECT_GT(timing.average_time, 0);
    EXPECT_GE(timing.min_time, 0);
    EXPECT_GE(timing.max_time, 0);
    EXPECT_GT(timing.throughput, 0);
    EXPECT_LE(timing.min_time, timing.average_time);
    EXPECT_GE(timing.max_time, timing.average_time);
}

// Test timing with memory tracking
TEST_F(PerformanceMetricsTest, TimeOperationWithMemory) {
    auto operation = []() {
        std::vector<int> data(1000, 42);
        for (auto& val : data) {
            val *= 2;
        }
    };

    MemoryStats mem_stats;
    TimingStats timing = PerformanceMetrics::time_operation_with_memory(operation, mem_stats, 10);

    EXPECT_GT(timing.average_time, 0);
    EXPECT_GE(mem_stats.current_memory, 0);
    EXPECT_GE(mem_stats.peak_memory, 0);
    EXPECT_GE(mem_stats.average_memory, 0);
}

// Test CPU cycle counting
TEST_F(PerformanceMetricsTest, TimeOperationCycles) {
    auto operation = []() {
        volatile int sum = 0;
        for (int i = 0; i < 100; ++i) {
            sum += i;
        }
    };

    CycleStats cycles = PerformanceMetrics::time_operation_cycles(operation, 10);

    EXPECT_GT(cycles.average_cycles, 0);
    EXPECT_GE(cycles.min_cycles, 0);
    EXPECT_GE(cycles.max_cycles, 0);
    EXPECT_LE(cycles.min_cycles, cycles.average_cycles);
    EXPECT_GE(cycles.max_cycles, cycles.average_cycles);
}

// Test combined measurement
TEST_F(PerformanceMetricsTest, MeasureOperation) {
    auto operation = []() {
        std::vector<double> data(100);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = std::sin(i * 0.1);
        }
    };

    auto stats = PerformanceMetrics::measure_operation(operation, 5);

    // Check timing stats
    EXPECT_GT(stats.timing.average_time, 0);
    EXPECT_GT(stats.timing.throughput, 0);

    // Check memory stats
    EXPECT_GE(stats.memory.current_memory, 0);
    EXPECT_GE(stats.memory.peak_memory, 0);
    EXPECT_GE(stats.memory.average_memory, 0);

    // Check cycle stats
    EXPECT_GT(stats.cycles.average_cycles, 0);
    EXPECT_GE(stats.cycles.min_cycles, 0);
    EXPECT_GE(stats.cycles.max_cycles, 0);
}

// Test with empty operation
TEST_F(PerformanceMetricsTest, EmptyOperation) {
    auto empty_op = []() { /* do nothing */ };

    TimingStats timing = PerformanceMetrics::time_operation(empty_op, 1);
    EXPECT_GE(timing.average_time, 0);

    CycleStats cycles = PerformanceMetrics::time_operation_cycles(empty_op, 1);
    EXPECT_GE(cycles.average_cycles, 0);
}

// Test multiple iterations consistency
TEST_F(PerformanceMetricsTest, MultipleIterations) {
    auto operation = []() {
        int x = 0;
        for (int i = 0; i < 10; ++i) {
            x += i * i;
        }
    };

    // Test with different iteration counts
    TimingStats timing_5 = PerformanceMetrics::time_operation(operation, 5);
    TimingStats timing_10 = PerformanceMetrics::time_operation(operation, 10);

    // Both should be positive
    EXPECT_GE(timing_5.average_time, 0);
    EXPECT_GE(timing_10.average_time, 0);

    // Results should be reasonably close (within 50% of each other)
    double ratio = timing_5.average_time / timing_10.average_time;
    EXPECT_TRUE(ratio > 0.5 || std::isnan(ratio));
    EXPECT_TRUE(ratio < 2.0 || std::isnan(ratio));
}