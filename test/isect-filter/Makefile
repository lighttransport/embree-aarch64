
# Variables
#
# - CXX
# - CXX_FLAGS
# - EMBREE_PATH

# Examples:
#
# - make
# - make CXX=g++
# - make CXX=clang++
# - make CXX=/usr/local/opt/llvm/bin/clang++ EMBREE_PATH=/opt/embree/embree-3.2.0.x86_64.macosx

SHELL := /bin/bash

CXX=clang++ -fsanitize=address
CXX_FLAGS=-std=c++11 -fsanitize=address -O0 -g -DTASKING_INTERNAL -DEMBREE_TARGET_SSE2 
LINK_FLAGS=
EMBREE_INC_PATH=../../include
EMBREE_LIB_PATH=../../build

all: test_isectfilter

main.o: main.cc
	$(CXX) $(CXX_FLAGS) -c $^ -I ${EMBREE_INC_PATH} -I. -o $@
noise.o: ../../tutorials/common/tutorial/noise.cpp
	$(CXX) $(CXX_FLAGS) -c $^ -I ${EMBREE_INC_PATH} -I. -o $@
constants.o: ../../common/math/constants.cpp
	$(CXX) $(CXX_FLAGS) -c $^ -I ${EMBREE_INC_PATH} -I. -o $@

test_isectfilter: main.o noise.o constants.o
	$(CXX) $^ -L ${EMBREE_LIB_PATH} ${LINK_FLAGS} -lembree3 -o $@

clean:
	rm -f a.out main *.png *.o

.PHONY: clean
