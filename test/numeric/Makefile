
# Variables
#
# - CXX
# - EXTRA_CXXFLAGS
# - EMBREE_LIB_PATH

# Examples:
#
# - make
# - make CXX=g++
# - make CXX=clang++
# - make CXX=/usr/local/opt/llvm/bin/clang++ EMBREE_LIB_PATH=/opt/embree/embree-3.2.0.x86_64.macosx

SHELL := /bin/bash

CXX ?= g++
CXXFLAGS = $(EXTRA_CXXFLAGS) -Wall -Wno-pedantic -std=c++11 -O0 -g -DEMBREE_TARGET_SSE2 -DTASKING_INTERNAL
LDFLAGS = $(EXTRA_LDFLAGS)
EMBREE_INC_PATH = ../../include
EMBREE_LIB_PATH ?= ../../build-aarch64

all: test_numeric

main.o: main.cc
	$(CXX) $(CXXFLAGS) -c $^ -I ${EMBREE_INC_PATH} -I. -o $@

test_numeric: main.o
	$(CXX) $^ $(LDFLAGS) -L ${EMBREE_LIB_PATH} -lembree3 -o $@

clean:
	rm -f a.out main *.png *.o

.PHONY: clean
