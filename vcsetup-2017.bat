rmdir /q /s build
mkdir build
cd build

cmake -G "Visual Studio 15 2017" -A x64 ^
  -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
  -DEMBREE_ISPC_SUPPORT=Off ^
  -DEMBREE_TASKING_SYSTEM=Internal ^
  -DEMBREE_TUTORIALS=Off ^
  -DEMBREE_MAX_ISA=SSE2 ^
  -DEMBREE_RAY_PACKETS=Off ^
  ..
