#!/bin/bash

builddir=`pwd`/build-llvm-mingw
curdir=`pwd`

rm -rf ${builddir}
mkdir ${builddir}

cd ${builddir} && cmake -DCMAKE_TOOLCHAIN_FILE=${curdir}/common/cmake/llvm-mingw-cross.cmake \
  -DCMAKE_INSTALL_PREFIX=$HOME/local/llvm-mingw-embree3 \
  -DEMBREE_ISPC_SUPPORT=Off \
  -DEMBREE_TASKING_SYSTEM=Internal \
  -DEMBREE_TUTORIALS=Off \
  -DEMBREE_STATIC_LIB=On \
  -DEMBREE_MAX_ISA=SSE2 \
  ..
