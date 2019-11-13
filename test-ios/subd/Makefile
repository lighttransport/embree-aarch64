
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

CXX=/Applications/Xcode.app/Contents/Developer/Toolchains/iOS12.4.xctoolchain/usr/bin/clang++
CXX_FLAGS=-arch arm64e -Wall -Wno-pedantic -std=c++17 -O3 -g -DTASKING_GCD
EMBREE_INC_PATH=../../include
EMBREE_LIB_PATH=../../build-aarch64-cross

all: test_subd

main.o: main.cc
	$(CXX) $(CXX_FLAGS) -c $^ -I ${EMBREE_INC_PATH} -I. -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS12.4.sdk -o $@

test_subd: main.o
	$(CXX) -arch arm64e $^ -L ${EMBREE_LIB_PATH} -lembree3 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS12.4.sdk -o $@

clean:
	rm -f a.out main *.png *.o test_subd

.PHONY: clean
