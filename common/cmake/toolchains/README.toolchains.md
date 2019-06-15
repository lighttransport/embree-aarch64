For cross-compiling to Android aarch64 / arm64 on Windows, configure the following:

```
cmake ^
-G Ninja ^
-DCMAKE_BUILD_TYPE=Release ^
-DCMAKE_SKIP_RPATH:BOOL=YES ^
-DCMAKE_INSTALL_PREFIX:PATH=../release/ninja-android-aarch64 ^
-DCMAKE_TOOLCHAIN_FILE=../../common/cmake/toolchains/android-aarch64.toolchain.cmake ^
-DEMBREE_MAX_ISA=SSE2 ^
-DEMBREE_ISPC_SUPPORT:BOOL=NO ^
-DEMBREE_TASKING_SYSTEM=Internal ^
-DEMBREE_TUTORIALS:BOOL=NO ^
-DEMBREE_RAY_PACKETS:BOOL=NO ^
-DEMBREE_BUILD_VERIFY:BOOL=NO ^
../..
```

For cross-compiling to Android x64 (e.g. for emulator and Pixelbook devices) on Windows, configure the following:

```
cmake ^
-G Ninja ^
-DCMAKE_BUILD_TYPE=Release ^
-DCMAKE_SKIP_RPATH:BOOL=YES ^
-DCMAKE_INSTALL_PREFIX:PATH=../release/ninja-android-x64 ^
-DCMAKE_TOOLCHAIN_FILE=../../common/cmake/toolchains/android-x64.toolchain.cmake ^
-DEMBREE_ISPC_SUPPORT:BOOL=NO ^
-DEMBREE_TASKING_SYSTEM=Internal ^
-DEMBREE_TUTORIALS:BOOL=NO ^
-DEMBREE_BUILD_VERIFY:BOOL=NO ^
../..
```

For cross-compiling to iOS aarch64 / arm64 on OSX, configure the following:

```
cmake \
-G Ninja \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX:PATH=../release/ninja-ios-arm64  \
-DEMBREE_MAX_ISA=SSE2 \
-DCMAKE_TOOLCHAIN_FILE=../../common/cmake/toolchains/ios.toolchain.cmake \
-DIOS_ARCH=arm64 \
-DPLATFORM_NAME=iphoneos \
-DIOS=1 \
../..
```

For cross-compiling to iOS simulator (x64) on OSX, configure the following:

```
cmake \
-G Ninja \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX:PATH=../release/ninja-ios-x64  \
-DCMAKE_TOOLCHAIN_FILE=../../common/cmake/toolchains/ios.toolchain.cmake \
-DIOS_ARCH=x86_64 \
-DPLATFORM_NAME=iphonesimulator \
-DIOS=1 \
../..
```
