#include "clwe/performance_metrics.hpp"
#include <chrono>
#include <algorithm>
#include <numeric>
#include <limits>

// Include platform-specific implementations
#if defined(__linux__)
#include "performance_metrics_linux.cpp"
#elif defined(__APPLE__)
#include "performance_metrics_macos.cpp"
#elif defined(_WIN32)
#include "performance_metrics_windows.cpp"
#endif

// ESP32-specific implementations
#ifdef ESP_PLATFORM
#include <esp_heap_caps.h>
#include <esp_cpu.h>

namespace clwe {

// ESP32-specific memory measurement
MemoryStats PerformanceMetrics::get_memory_usage_impl() {
    size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    size_t total_heap = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    size_t used_heap = total_heap - free_heap;

    // Simplified: current = used, peak = used (no tracking), average = used
    return {used_heap, used_heap, used_heap};
}

// ESP32-specific CPU cycle counting
uint64_t PerformanceMetrics::get_cpu_cycles_impl() {
    return esp_cpu_get_ccount();
}

} // namespace clwe

#endif

namespace clwe {

// Get current memory usage
MemoryStats PerformanceMetrics::get_memory_usage() {
    return get_memory_usage_impl();
}

// Time operation with memory tracking
TimingStats PerformanceMetrics::time_operation_with_memory(
    const std::function<void()>& operation,
    MemoryStats& memory_stats,
    int iterations
) {
    std::vector<double> times;
    std::vector<size_t> memory_usages;

    size_t min_memory = std::numeric_limits<size_t>::max();
    size_t max_memory = 0;
    size_t total_memory = 0;

    for (int i = 0; i < iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        operation();
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        times.push_back(static_cast<double>(duration.count()));

        // Get memory usage after operation
        MemoryStats current_mem = get_memory_usage();
        memory_usages.push_back(current_mem.current_memory);

        min_memory = std::min(min_memory, current_mem.current_memory);
        max_memory = std::max(max_memory, current_mem.current_memory);
        total_memory += current_mem.current_memory;
    }

    double total_time = std::accumulate(times.begin(), times.end(), 0.0);
    double avg_time = total_time / iterations;
    double min_time = *std::min_element(times.begin(), times.end());
    double max_time = *std::max_element(times.begin(), times.end());
    double throughput = 1000000.0 / avg_time;  // operations per second

    memory_stats.current_memory = memory_usages.back();
    memory_stats.peak_memory = max_memory;
    memory_stats.average_memory = total_memory / iterations;

    return {total_time, avg_time, min_time, max_time, throughput};
}

// Time operation with CPU cycle counting
CycleStats PerformanceMetrics::time_operation_cycles(
    const std::function<void()>& operation,
    int iterations
) {
    std::vector<uint64_t> cycles;

    for (int i = 0; i < iterations; ++i) {
        uint64_t start_cycles = get_cpu_cycles_impl();
        operation();
        uint64_t end_cycles = get_cpu_cycles_impl();
        cycles.push_back(end_cycles - start_cycles);
    }

    uint64_t total_cycles = std::accumulate(cycles.begin(), cycles.end(), 0ULL);
    uint64_t avg_cycles = total_cycles / iterations;
    uint64_t min_cycles = *std::min_element(cycles.begin(), cycles.end());
    uint64_t max_cycles = *std::max_element(cycles.begin(), cycles.end());

    return {total_cycles, avg_cycles, min_cycles, max_cycles};
}

// High-precision timing only
TimingStats PerformanceMetrics::time_operation(
    const std::function<void()>& operation,
    int iterations
) {
    std::vector<double> times;

    for (int i = 0; i < iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        operation();
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        times.push_back(static_cast<double>(duration.count()));
    }

    double total_time = std::accumulate(times.begin(), times.end(), 0.0);
    double avg_time = total_time / iterations;
    double min_time = *std::min_element(times.begin(), times.end());
    double max_time = *std::max_element(times.begin(), times.end());
    double throughput = 1000000.0 / avg_time;

    return {total_time, avg_time, min_time, max_time, throughput};
}

// Combined measurement
PerformanceMetrics::CombinedStats PerformanceMetrics::measure_operation(
    const std::function<void()>& operation,
    int iterations
) {
    CombinedStats result;

    // Measure timing and memory together
    result.timing = time_operation_with_memory(operation, result.memory, iterations);

    // Measure cycles separately
    result.cycles = time_operation_cycles(operation, iterations);

    return result;
}

} // namespace clwe