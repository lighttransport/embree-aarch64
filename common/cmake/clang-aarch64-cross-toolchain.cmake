# Based on https://stackoverflow.com/questions/54539682/how-to-set-up-cmake-to-cross-compile-with-clang-for-arm-embedded-on-windows
set(CMAKE_CROSSCOMPILING TRUE)
SET(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(DEFINED ENV{GCC_ARM_TOOLCHAIN})
    set(GCC_ARM_TOOLCHAIN $ENV{GCC_ARM_TOOLCHAIN})
else()
    # Assume `/usr/aarch64-linux-gnu/` exists
    # (e.g. though `sudo apt install gcc-aarch64-linux-gnu` on Ubuntu)
    set(GCC_ARM_TOOLCHAIN "/usr")
endif()

# Base directory to `/bin/clang` and `/bin/clang++`
if(DEFINED ENV{CLANG_TOOLCHAIN})
    set(CLANG_TOOLCHAIN $ENV{CLANG_TOOLCHAIN})
else()
    set(CLANG_TOOLCHAIN "/usr")
endif()

# Clang target triple
SET(TARGET_TRIPLE aarch64-linux-gnu)

# specify the cross compiler
# TODO(LTE): Read clang path from environment variable
SET(CMAKE_C_COMPILER_TARGET ${TARGET_TRIPLE})
SET(CMAKE_C_COMPILER ${CLANG_TOOLCHAIN}/bin/clang)
SET(CMAKE_CXX_COMPILER_TARGET ${TARGET_TRIPLE})
SET(CMAKE_CXX_COMPILER ${CLANG_TOOLCHAIN}/bin/clang++)
SET(CMAKE_ASM_COMPILER_TARGET ${TARGET_TRIPLE})
SET(CMAKE_ASM_COMPILER ${CLANG_TOOLCHAIN}/bin/clang++)

# Don't run the linker on compiler check
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)


# or we could use `-B` for clang
# this flag is required to find libc(glibc) headers, crtbegin.o, etc
set(CMAKE_C_FLAGS_INIT " --gcc-toolchain=${GCC_ARM_TOOLCHAIN}")
set(CMAKE_CXX_FLAGS_INIT " --gcc-toolchain=${GCC_ARM_TOOLCHAIN} ")

# C/C++ toolchain
set(GCC_ARM_SYSROOT "${GCC_ARM_TOOLCHAIN}/${TARGET_TRIPLE}")
set(CMAKE_SYSROOT ${GCC_ARM_SYSROOT})
#set(CMAKE_FIND_ROOT_PATH ${GCC_ARM_SYSROOT})

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
