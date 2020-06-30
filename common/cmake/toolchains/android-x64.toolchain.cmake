# Copyright (C) 2018 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

SET(CMAKE_SYSTEM_NAME Linux)

# Android
SET(API_LEVEL 21)

# architecture
SET(ARCH x86_64-linux-android)
SET(DIST_ARCH x86_64)

# toolchain
STRING(TOLOWER ${CMAKE_HOST_SYSTEM_NAME} HOST_NAME_L)
SET(TOOLCHAIN $ENV{ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/${HOST_NAME_L}-x86_64/)

# specify the cross compiler
SET(COMPILER_SUFFIX)
SET(TOOL_SUFFIX)
IF(WIN32)
  SET(COMPILER_SUFFIX ".cmd")
  SET(TOOL_SUFFIX     ".exe")
ENDIF()
SET(CMAKE_C_COMPILER   ${TOOLCHAIN}/bin/${ARCH}${API_LEVEL}-clang${COMPILER_SUFFIX})
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN}/bin/${ARCH}${API_LEVEL}-clang++${COMPILER_SUFFIX})
SET(CMAKE_AR           ${TOOLCHAIN}/bin/${ARCH}-ar${TOOL_SUFFIX}  CACHE FILEPATH "Archiver")
SET(CMAKE_RANLIB       ${TOOLCHAIN}/bin/${ARCH}-ranlib${TOOL_SUFFIX})
SET(CMAKE_STRIP        ${TOOLCHAIN}/bin/${ARCH}-strip${TOOL_SUFFIX})

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH  ${TOOLCHAIN}/sysroot)

# compiler and linker flags
SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -DANDROID -fPIE" CACHE STRING "Toolchain CFLAGS")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS}" CACHE STRING "Toolchain CXXFLAGS")
SET(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,--no-warn-mismatch -L${TOOLCHAIN}/x86_64-linux-android/lib64/ -static-libstdc++ -fPIE -pie" CACHE STRING "Toolchain LDFLAGS")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-warn-mismatch -L${TOOLCHAIN}/x86_64-linux-android/lib64/ -static-libstdc++" CACHE STRING "Toolchain LDFLAGS")
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

SET(ANDROID TRUE)
