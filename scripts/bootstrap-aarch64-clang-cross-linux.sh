#!/bin/bash

curdir=`pwd`

CMAKE_BIN=cmake
BUILD_DIR=build-aarch64-cross-clang

rm -rf $BUILD_DIR
mkdir $BUILD_DIR

cd $BUILD_DIR

# If you want to change compiler, edit `clang-aarch64-cross-toolchain.cmake`
# `EMBREE_USE_LLD` must be enabled otherwise liking error will happen
$CMAKE_BIN \
  -DCMAKE_TOOLCHAIN_FILE=${curdir}/common/cmake/clang-aarch64-cross-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DEMBREE_ARM=On \
  -DEMBREE_ADDRESS_SANITIZER=Off \
  -DCMAKE_INSTALL_PREFIX=$HOME/local/embree3 \
  -DEMBREE_ISPC_SUPPORT=Off \
  -DEMBREE_TASKING_SYSTEM=Internal \
  -DEMBREE_TUTORIALS=Off \
  -DEMBREE_MAX_ISA=SSE2 \
  -DEMBREE_RAY_PACKETS=On \
  -DEMBREE_RAY_PACKETS=On \
  -DEMBREE_USE_LLD=On \
  ..

cd ..
