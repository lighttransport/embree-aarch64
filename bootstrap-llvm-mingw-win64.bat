rmdir /s /q build-llvm-mingw
mkdir build-llvm-mingw

cd build-llvm-mingw

cmake.exe -DCMAKE_TOOLCHAIN_FILE=./common/cmake/llvm-mingw-win64.cmake ^
  -G Ninja ^
  -DCMAKE_INSTALL_PREFIX=%HOME%/local/llvm-mingw-embree3 ^
  -DEMBREE_ISPC_SUPPORT=Off ^
  -DEMBREE_TASKING_SYSTEM=Internal ^
  -DEMBREE_TUTORIALS=Off ^
  -DEMBREE_STATIC_LIB=On ^
  -DEMBREE_MAX_ISA=SSE2 ^
  ..
