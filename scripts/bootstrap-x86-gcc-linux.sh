#!/bin/bash

CMAKE_BIN=cmake

rm -rf build
mkdir build

cd build

$CMAKE_BIN \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DEMBREE_ADDRESS_SANITIZER=Off \
  -DCMAKE_INSTALL_PREFIX=$HOME/local/x86/embree3 \
  -DEMBREE_ISPC_SUPPORT=Off \
  -DEMBREE_TASKING_SYSTEM=Internal \
  -DEMBREE_TUTORIALS=Off \
  -DEMBREE_MAX_ISA=SSE2 \
  -DEMBREE_RAY_PACKETS=Off \
  ..

cd ..
