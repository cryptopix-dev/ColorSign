# CMake generated Testfile for 
# Source directory: C:/Users/abhin/Documents/GitHub/ColorSign/windows/sign
# Build directory: C:/Users/abhin/Documents/GitHub/ColorSign/windows/sign/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(BenchmarkTest "C:/Users/abhin/Documents/GitHub/ColorSign/windows/sign/build/benchmark_color_sign_timing.exe")
set_tests_properties(BenchmarkTest PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/abhin/Documents/GitHub/ColorSign/windows/sign/CMakeLists.txt;118;add_test;C:/Users/abhin/Documents/GitHub/ColorSign/windows/sign/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
subdirs("tests")
