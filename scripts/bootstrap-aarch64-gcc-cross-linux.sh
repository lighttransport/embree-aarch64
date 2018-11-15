#!/bin/bash

CMAKE_BIN=cmake

rm -rf build-aarch64-cross

$CMAKE_BIN \
  -DEMBREE_TARGET_ARCH=aarch64 \
  -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc-5 \
  -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++-5 \
  -DEMBREE_ISPC_SUPPORT=Off \
  -DEMBREE_TASKING_SYSTEM=Internal \
  -DEMBREE_TUTORIALS=Off \
  -DEMBREE_MAX_ISA=SSE2 \
  -DEMBREE_RAY_PACKETS=Off \
  -Bbuild-aarch64-cross -H.
