#!/bin/bash

builddir=`pwd`/build-mingw64
curdir=`pwd`

rm -rf ${builddir}
mkdir ${builddir}

cd ${builddir} && cmake -DCMAKE_TOOLCHAIN_FILE=${curdir}/common/cmake/mingw64-cross.cmake \
  -DCMAKE_INSTALL_PREFIX=$HOME/local/mingw-embree3 \
  -DEMBREE_ISPC_SUPPORT=Off \
  -DEMBREE_TASKING_SYSTEM=Internal \
  -DEMBREE_TUTORIALS=Off \
  -DEMBREE_STATIC_LIB=On \
  -DEMBREE_MAX_ISA=SSE2 \
  ..
