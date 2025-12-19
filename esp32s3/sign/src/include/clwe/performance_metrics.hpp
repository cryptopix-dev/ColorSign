#ifndef PERFORMANCE_METRICS_HPP
#define PERFORMANCE_METRICS_HPP

#include <cstdint>
#include <functional>
#include <vector>

namespace clwe {

// Memory usage statistics
struct MemoryStats {
    size_t current_memory;  // Current memory usage in bytes
    size_t peak_memory;     // Peak memory usage in bytes
    size_t average_memory;  // Average memory usage in bytes
};

// CPU cycle statistics
struct CycleStats {
    uint64_t total_cycles;    // Total CPU cycles
    uint64_t average_cycles;  // Average CPU cycles per operation
    uint64_t min_cycles;      // Minimum CPU cycles
    uint64_t max_cycles;      // Maximum CPU cycles
};

// High-precision timing statistics
struct TimingStats {
    double total_time;      // Total time in microseconds
    double average_time;    // Average time per operation in microseconds
    double min_time;        // Minimum time in microseconds
    double max_time;        // Maximum time in microseconds
    double throughput;      // Operations per second
};

// Performance measurement class
class PerformanceMetrics {
public:
    // Get current memory usage
    static MemoryStats get_memory_usage();

    // Time an operation with memory tracking
    static TimingStats time_operation_with_memory(
        const std::function<void()>& operation,
        MemoryStats& memory_stats,
        int iterations = 100
    );

    // Time an operation with CPU cycle counting
    static CycleStats time_operation_cycles(
        const std::function<void()>& operation,
        int iterations = 100
    );

    // High-precision timing only
    static TimingStats time_operation(
        const std::function<void()>& operation,
        int iterations = 100
    );

    // Combined measurement (timing + memory + cycles)
    struct CombinedStats {
        TimingStats timing;
        MemoryStats memory;
        CycleStats cycles;
    };

    static CombinedStats measure_operation(
        const std::function<void()>& operation,
        int iterations = 100
    );

private:
    // Platform-specific implementations
    static MemoryStats get_memory_usage_impl();
    static uint64_t get_cpu_cycles_impl();
};

} // namespace clwe

#endif // PERFORMANCE_METRICS_HPP