#!/bin/bash

# aarch64 linux native build

CMAKE_BIN=cmake

rm -rf build-aarch64
mkdir build-aarch64

cd build-aarch64

# CC and CXX are set by Travis
# Disable ASAN for a while to avoid Travis job timeout when running `verify` test.

$CMAKE_BIN \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DEMBREE_ARM=On \
  -DEMBREE_ADDRESS_SANITIZER=Off \
  -DCMAKE_INSTALL_PREFIX=$HOME/local/embree3 \
  -DEMBREE_ISPC_SUPPORT=Off \
  -DEMBREE_TASKING_SYSTEM=Internal \
  -DEMBREE_TUTORIALS=Off \
  -DEMBREE_MAX_ISA=SSE2 \
  -DEMBREE_RAY_PACKETS=Off \
  ..

cd ..
