
# Variables
#
# - CXX
# - EXTRA_CXXFLAGS
# - EXTRA_LDFLAGS
# - EMBREE_LIB_PATH

# Examples:
#
# - make
# - make CXX=g++
# - make CXX=clang++
# - make CXX=/usr/local/opt/llvm/bin/clang++ EMBREE_LIB_PATH=/opt/embree/embree-3.2.0.x86_64.macosx

SHELL := /bin/bash

CXX ?= clang++
CXX_FLAGS=$(EXTRA_CXXFLAGS) -std=c++11 -O0 -g -DTASKING_INTERNAL
LINK_FLAGS=$(EXTRA_LDFLGS)
EMBREE_INC_PATH=../../include
EMBREE_LIB_PATH ?= ../../build-aarch64

all: test_disp

main.o: main.cc
	$(CXX) $(CXX_FLAGS) -c $^ -I ${EMBREE_INC_PATH} -I. -o $@
noise.o: ../../tutorials/common/tutorial/noise.cpp
	$(CXX) $(CXX_FLAGS) -c $^ -I ${EMBREE_INC_PATH} -I. -o $@

test_disp: main.o noise.o
	$(CXX) $^ -L ${EMBREE_LIB_PATH} ${LINK_FLAGS} -lembree3 -o $@

clean:
	rm -f a.out main *.png *.o

.PHONY: clean
