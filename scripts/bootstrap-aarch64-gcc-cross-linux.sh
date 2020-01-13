#!/bin/bash

CMAKE_BIN=cmake
BUILD_DIR=build-aarch64-cross

rm -rf $BUILD_DIR
mkdir $BUILD_DIR

cd $BUILD_DIR

$CMAKE_BIN \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DEMBREE_ARM=On \
  -DEMBREE_ADDRESS_SANITIZER=On \
  -DCMAKE_INSTALL_PREFIX=$HOME/local/embree3 \
  -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc-8 \
  -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++-8 \
  -DEMBREE_ISPC_SUPPORT=Off \
  -DEMBREE_TASKING_SYSTEM=Internal \
  -DEMBREE_TUTORIALS=Off \
  -DEMBREE_MAX_ISA=SSE2 \
  -DEMBREE_RAY_PACKETS=On \
  ..

cd ..
