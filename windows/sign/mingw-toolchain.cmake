# MinGW toolchain file for ColorSign Windows build

# Specify the compilers
set(CMAKE_C_COMPILER "C:/ProgramData/mingw64/mingw64/bin/gcc.exe" CACHE FILEPATH "C compiler")
set(CMAKE_CXX_COMPILER "C:/ProgramData/mingw64/mingw64/bin/g++.exe" CACHE FILEPATH "C++ compiler")

# Set OpenSSL path for MinGW
set(OPENSSL_ROOT_DIR "C:/ProgramData/mingw64/mingw64/opt" CACHE PATH "OpenSSL root directory")

# Set compiler flags
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Require C++ standard")

# Windows-specific definitions
add_definitions(-D_MINGW -DWIN32 -D_WIN32)