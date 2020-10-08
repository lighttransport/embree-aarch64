#!/bin/bash

# aarch64 linux native build

CMAKE_BIN=cmake

rm -rf build-aarch64-neon-x2
mkdir build-aarch64-neon-x2

cd build-aarch64-neon-x2

$CMAKE_BIN \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DEMBREE_ARM=On \
  -DCMAKE_INSTALL_PREFIX=$HOME/local/embree3 \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DEMBREE_ISPC_SUPPORT=Off \
  -DEMBREE_TASKING_SYSTEM=Internal \
  -DEMBREE_TUTORIALS=Off \
  -DEMBREE_RAY_PACKETS=Off \
  -DEMBREE_MAX_ISA=AVX2 \
  -DEMBREE_NEON_AVX2_EMULATION=ON \
  ..

cd ..
