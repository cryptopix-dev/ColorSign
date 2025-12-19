#include <esp_log.h>
#include <vector>
#include <functional>
#include <algorithm>
#include <clwe/clwe.hpp>
#include <clwe/color_kem.hpp>
#include <clwe/cpu_features.hpp>
#include <clwe/performance_metrics.hpp>

using namespace clwe;

static const char *TAG = "ColorKEM_Benchmark";

void benchmark_security_level(int security_level, int iterations = 10) {
    ESP_LOGI(TAG, "Security Level: %d-bit", security_level);
    ESP_LOGI(TAG, "=====================================");

    clwe::CLWEParameters params(security_level);
    clwe::ColorKEM kem(params);

    // Generate keys and ciphertext for size measurements
    auto keypair = kem.keygen();
    auto public_key = keypair.first;
    auto private_key = keypair.second;
    auto encap_result = kem.encapsulate(public_key);
    auto ciphertext = encap_result.first;
    auto shared_secret = encap_result.second;

    // Size calculations
    size_t public_key_size = public_key.serialize().size();
    size_t private_key_size = private_key.serialize().size();
    size_t ciphertext_size = ciphertext.serialize().size();
    size_t shared_secret_size = sizeof(ColorValue);

    // Memory and timing measurements using new API
    clwe::MemoryStats keygen_mem, encap_mem, decap_mem;
    clwe::TimingStats keygen_timing = clwe::PerformanceMetrics::time_operation_with_memory([&]() {
        auto kp = kem.keygen();
        auto pk = kp.first;
        auto sk = kp.second;
    }, keygen_mem, iterations);

    clwe::TimingStats encap_timing = clwe::PerformanceMetrics::time_operation_with_memory([&]() {
        auto er = kem.encapsulate(public_key);
        auto ct = er.first;
        auto ss = er.second;
    }, encap_mem, iterations);

    clwe::TimingStats decap_timing = clwe::PerformanceMetrics::time_operation_with_memory([&]() {
        auto recovered = kem.decapsulate(public_key, private_key, ciphertext);
    }, decap_mem, iterations);

    // CPU cycle measurements
    clwe::CycleStats keygen_cycles = clwe::PerformanceMetrics::time_operation_cycles([&]() {
        auto kp = kem.keygen();
        auto pk = kp.first;
        auto sk = kp.second;
    }, iterations);

    clwe::CycleStats encap_cycles = clwe::PerformanceMetrics::time_operation_cycles([&]() {
        auto er = kem.encapsulate(public_key);
        auto ct = er.first;
        auto ss = er.second;
    }, iterations);

    clwe::CycleStats decap_cycles = clwe::PerformanceMetrics::time_operation_cycles([&]() {
        auto recovered = kem.decapsulate(public_key, private_key, ciphertext);
    }, iterations);

    // Calculate bandwidth (bytes transferred per second)
    double keygen_bandwidth = (public_key_size + private_key_size) / (keygen_timing.average_time / 1000000.0);
    double encap_bandwidth = (ciphertext_size + shared_secret_size) / (encap_timing.average_time / 1000000.0);
    double decap_bandwidth = (ciphertext_size + shared_secret_size) / (decap_timing.average_time / 1000000.0);

    // Overall calculations
    double total_kem_time = keygen_timing.average_time + encap_timing.average_time + decap_timing.average_time;
    double throughput = 1000000.0 / total_kem_time;
    uint64_t total_cycles = keygen_cycles.average_cycles + encap_cycles.average_cycles + decap_cycles.average_cycles;
    double cycles_per_second = total_cycles / (total_kem_time / 1000000.0);

    // Memory statistics
    size_t total_peak_memory = std::max({keygen_mem.peak_memory, encap_mem.peak_memory, decap_mem.peak_memory});
    size_t avg_memory = (keygen_mem.average_memory + encap_mem.average_memory + decap_mem.average_memory) / 3;

    // Display results
    ESP_LOGI(TAG, "=== TIMING METRICS ===");
    ESP_LOGI(TAG, "Key Generation:     %.2f μs", keygen_timing.average_time);
    ESP_LOGI(TAG, "Encapsulation:      %.2f μs", encap_timing.average_time);
    ESP_LOGI(TAG, "Decapsulation:      %.2f μs", decap_timing.average_time);
    ESP_LOGI(TAG, "Total KEM Time:     %.2f μs", total_kem_time);
    ESP_LOGI(TAG, "Throughput:         %.2f operations/second", throughput);

    ESP_LOGI(TAG, "=== CPU CYCLE METRICS ===");
    ESP_LOGI(TAG, "KeyGen Cycles:      %llu", keygen_cycles.average_cycles);
    ESP_LOGI(TAG, "Encap Cycles:       %llu", encap_cycles.average_cycles);
    ESP_LOGI(TAG, "Decap Cycles:       %llu", decap_cycles.average_cycles);
    ESP_LOGI(TAG, "Total Cycles:       %llu", total_cycles);
    ESP_LOGI(TAG, "Cycles/Second:      %.2f", cycles_per_second);

    ESP_LOGI(TAG, "=== MEMORY USAGE METRICS ===");
    ESP_LOGI(TAG, "Peak Memory:        %.2f KB", total_peak_memory / 1024.0);
    ESP_LOGI(TAG, "Average Memory:     %.2f KB", avg_memory / 1024.0);

    ESP_LOGI(TAG, "=== STORAGE REQUIREMENTS ===");
    ESP_LOGI(TAG, "Public Key Size:    %zu bytes", public_key_size);
    ESP_LOGI(TAG, "Private Key Size:   %zu bytes", private_key_size);
    ESP_LOGI(TAG, "Ciphertext Size:    %zu bytes", ciphertext_size);
    ESP_LOGI(TAG, "Shared Secret Size: %zu bytes", shared_secret_size);

    ESP_LOGI(TAG, "=== BANDWIDTH METRICS ===");
    ESP_LOGI(TAG, "KeyGen Bandwidth:   %.2f KB/s", keygen_bandwidth / 1024.0);
    ESP_LOGI(TAG, "Encap Bandwidth:    %.2f KB/s", encap_bandwidth / 1024.0);
    ESP_LOGI(TAG, "Decap Bandwidth:    %.2f KB/s", decap_bandwidth / 1024.0);

    ESP_LOGI(TAG, "=== PERFORMANCE BREAKDOWN ===");
    ESP_LOGI(TAG, "Time Distribution:");
    ESP_LOGI(TAG, "  KeyGen: %.2f%%", (keygen_timing.average_time / total_kem_time * 100));
    ESP_LOGI(TAG, "  Encap:  %.2f%%", (encap_timing.average_time / total_kem_time * 100));
    ESP_LOGI(TAG, "  Decap:  %.2f%%", (decap_timing.average_time / total_kem_time * 100));

    ESP_LOGI(TAG, "Cycle Distribution:");
    ESP_LOGI(TAG, "  KeyGen: %.2f%%", (keygen_cycles.average_cycles / (double)total_cycles * 100));
    ESP_LOGI(TAG, "  Encap:  %.2f%%", (encap_cycles.average_cycles / (double)total_cycles * 100));
    ESP_LOGI(TAG, "  Decap:  %.2f%%", (decap_cycles.average_cycles / (double)total_cycles * 100));
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "🎨 CLWE Color KEM Timing Benchmark");
    ESP_LOGI(TAG, "===================================");

    CPUFeatures features = CPUFeatureDetector::detect();
    ESP_LOGI(TAG, "CPU: %s", features.to_string().c_str());

    std::vector<int> security_levels = {512, 768, 1024};

    for (int level : security_levels) {
        benchmark_security_level(level);
    }

    ESP_LOGI(TAG, "Benchmark completed successfully!");
}