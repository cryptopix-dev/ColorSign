# MinGW toolchain file for ColorSign Windows build

# Allow MINGW_ROOT to be specified, default to Msys2 location if exists
if(NOT DEFINED MINGW_ROOT)
    if(EXISTS "C:/msys64/mingw64")
        set(MINGW_ROOT "C:/msys64/mingw64" CACHE PATH "MinGW Root Directory")
    elseif(EXISTS "C:/MinGW")
        set(MINGW_ROOT "C:/MinGW" CACHE PATH "MinGW Root Directory")
    else()
        # Try to find from GCC in PATH
        find_program(GCC_EXECUTABLE gcc)
        if(GCC_EXECUTABLE)
            get_filename_component(GCC_BIN_DIR ${GCC_EXECUTABLE} DIRECTORY)
            get_filename_component(MINGW_ROOT ${GCC_BIN_DIR} DIRECTORY)
        endif()
    endif()
endif()

if(NOT MINGW_ROOT)
    message(FATAL_ERROR "Could not find MinGW root. Please set MINGW_ROOT.")
endif()

# Specify the compilers
set(CMAKE_C_COMPILER "${MINGW_ROOT}/bin/gcc.exe" CACHE FILEPATH "C compiler")
set(CMAKE_CXX_COMPILER "${MINGW_ROOT}/bin/g++.exe" CACHE FILEPATH "C++ compiler")

# Set OpenSSL path for MinGW
set(OPENSSL_ROOT_DIR "${MINGW_ROOT}" CACHE PATH "OpenSSL root directory")

# Add to search prefixes
list(APPEND CMAKE_PREFIX_PATH "${MINGW_ROOT}")
list(APPEND CMAKE_FIND_ROOT_PATH "${MINGW_ROOT}")

# Set compiler flags
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Require C++ standard")

# Windows-specific definitions
add_definitions(-D_MINGW -DWIN32 -D_WIN32)