echo off
rem Set path to llvm-mingw with LLVM_MIGNW_DIR(See `common/cmake/llvm-mingw-win64.cmake` for the default path)
set LLVM_MINGW_DIR=D:/local/llvm-mingw

echo on



rmdir /s /q build-llvm-mingw
mkdir build-llvm-mingw

cd build-llvm-mingw && cmake.exe -S .. ^
  -DCMAKE_TOOLCHAIN_FILE=./common/cmake/llvm-mingw-win64.cmake ^
  -G Ninja ^
  -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
  -DCMAKE_INSTALL_PREFIX=%HOME%/local/llvm-mingw-embree3 ^
  -DEMBREE_ADDRESS_SANITIZER=On ^
  -DEMBREE_ISPC_SUPPORT=Off ^
  -DEMBREE_TASKING_SYSTEM=Internal ^
  -DEMBREE_TUTORIALS=Off ^
  -DEMBREE_STATIC_LIB=On ^
  -DEMBREE_MAX_ISA=SSE2
