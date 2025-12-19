# Tests CMakeLists.txt

# Include directories for tests
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/include/clwe)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/core)

# Test executables
add_executable(test_color_value test_color_value.cpp)
target_link_libraries(test_color_value PRIVATE clwe_macos gtest_main)

add_executable(test_clwe_parameters test_clwe_parameters.cpp)
target_link_libraries(test_clwe_parameters PRIVATE clwe_macos gtest_main)

add_executable(test_color_value test_color_value.cpp)
target_link_libraries(test_color_value PRIVATE clwe_macos gtest_main)

add_executable(test_clwe_parameters test_clwe_parameters.cpp)
target_link_libraries(test_clwe_parameters PRIVATE clwe_macos gtest_main)

add_executable(test_utils test_utils.cpp)
target_link_libraries(test_utils PRIVATE clwe_macos gtest_main)

add_executable(test_ntt_engine test_ntt_engine.cpp)
target_link_libraries(test_ntt_engine PRIVATE clwe_macos gtest_main)

add_executable(test_sampling test_sampling.cpp)
target_link_libraries(test_sampling PRIVATE clwe_macos gtest_main)

add_executable(test_performance_metrics test_performance_metrics.cpp)
target_link_libraries(test_performance_metrics PRIVATE clwe_macos gtest_main)

# Fuzzing targets (only if fuzzing is enabled)
if(ENABLE_FUZZING)
    add_executable(fuzz_edge_cases fuzz/fuzz_edge_cases.cpp)
    target_link_libraries(fuzz_edge_cases PRIVATE clwe_macos)
endif()

# Register tests
add_test(NAME ColorValueTests COMMAND test_color_value)
add_test(NAME CLWEParametersTests COMMAND test_clwe_parameters)
add_test(NAME UtilsTests COMMAND test_utils)
add_test(NAME NTTEngineTests COMMAND test_ntt_engine)
add_test(NAME SamplingTests COMMAND test_sampling)
add_test(NAME PerformanceMetricsTests COMMAND test_performance_metrics)