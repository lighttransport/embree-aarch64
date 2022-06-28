## Copyright 2009-2020 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

MACRO(_SET_IF_EMPTY VAR VALUE)
  IF(NOT ${VAR})
    SET(${VAR} "${VALUE}")
  ENDIF()
ENDMACRO()

IF (EMBREE_ARM)
   # No thing to declare.
ELSE ()
  # for `thread` keyword
  _SET_IF_EMPTY(FLAGS_SSE2  "-fms-extensions -msse -msse2 -mno-sse4.2")
  _SET_IF_EMPTY(FLAGS_SSE42 "-msse4.2")
  _SET_IF_EMPTY(FLAGS_AVX   "-mavx")
  _SET_IF_EMPTY(FLAGS_AVX2  "-mf16c -mavx2 -mfma -mlzcnt -mbmi -mbmi2")
  _SET_IF_EMPTY(FLAGS_AVX512KNL "-march=knl")
  _SET_IF_EMPTY(FLAGS_AVX512SKX "-march=skx")
ENDIF ()

IF (WIN32)

  OPTION(EMBREE_ADDRESS_SANITIZER "Enabled CLANG address sanitizer." OFF)

  IF (MINGW) # Assume llvm-mingw

    IF (EMBREE_ADDRESS_SANITIZER)
      INCLUDE(CheckCXXCompilerFlag)

      set(_CLANG_ASAN_CXX_FLAGS "-fsanitize=address ")

      check_cxx_compiler_flag("-fsanitize-address-use-after-scope" HAS_SANITIZE_ADDRESS_USE_AFTER_SCOPE)
      if (HAS_SANITIZE_ADDRESS_USE_AFTER_SCOPE)
        string(APPEND _CLANG_ASAN_CXX_FLAGS " -fsanitize-address-use-after-scope ")
      endif()

      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_CLANG_ASAN_CXX_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls")
    ENDIF()


    if (EMBREE_USE_PARENT_PROJECT_COMPILER_FLAGS)
      # use system provided compiler options and optimization flag
    else (EMBREE_USE_PARENT_PROJECT_COMPILER_FLAGS)

      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")                       # enables most warnings
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat -Wformat-security")  # enables string format vulnerability warnings
      #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE")                     # enables support for more secure position independent execution
      #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")                       # generate position independent code suitable for shared libraries
      #SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fPIC")                       # generate position independent code suitable for shared libraries
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")                # enables C++11 features
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")         # makes all symbols hidden by default
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden") # makes all inline symbols hidden by default
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")        # disables strict aliasing rules
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-tree-vectorize")         # disable auto vectorizer
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2")         # perform extra security checks for some standard library calls

      IF (EMBREE_STACK_PROTECTOR)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")           # protects against return address overrides
      ENDIF()

      SET(CMAKE_CXX_FLAGS_DEBUG "")
      SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")              # generate debug information
      SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")         # enable assertions
      SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTBB_USE_DEBUG") # configure TBB in debug mode
      SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3")             # enable full optimizations

      SET(CMAKE_CXX_FLAGS_RELEASE "")
      SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")     # disable assertions
      SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")          # enable full optimizations

      SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
      SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g")              # generate debug information
      SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DNDEBUG")        # disable assertions
      SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3")             # enable full optimizations

      IF (NOT EMBREE_ADDRESS_SANITIZER) # for address sanitizer this causes link errors
        SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined") # issues link error for undefined symbols in shared library
        SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -z noexecstack")     # we do not need an executable stack
        SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -z relro -z now")    # re-arranges data sections to increase security
        SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z noexecstack")           # we do not need an executable stack
        SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z relro -z now")          # re-arranges data sections to increase security
        SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")                     # enables position independent execution for executable
      ENDIF()

    endif (EMBREE_USE_PARENT_PROJECT_COMPILER_FLAGS)

    MACRO(DISABLE_STACK_PROTECTOR_FOR_FILE file)
      IF (EMBREE_STACK_PROTECTOR)
        SET_SOURCE_FILES_PROPERTIES(${file} PROPERTIES COMPILE_FLAGS "-fno-stack-protector")
      ENDIF()
    ENDMACRO()

  ELSE (MINGW)
    # Assume MSVC or clang-cl
    SET(COMMON_CXX_FLAGS "")
    SET(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} /EHsc")        # catch C++ exceptions only and extern "C" functions never throw a C++ exception
#    SET(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} /MP")          # compile source files in parallel
    SET(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} /GR")          # enable runtime type information (on by default)
    SET(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -Xclang -fcxx-exceptions") # enable C++ exceptions in Clang
    SET(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} /w")          # disable all warnings
    SET(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} /Gy")          # package individual functions
    IF (EMBREE_STACK_PROTECTOR)
      SET(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} /GS")          # protects against return address overrides
    ELSE()
      SET(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} /GS-")          # do not protect against return address overrides
    ENDIF()
    MACRO(DISABLE_STACK_PROTECTOR_FOR_FILE file)
      IF (EMBREE_STACK_PROTECTOR)
        SET_SOURCE_FILES_PROPERTIES(${file} PROPERTIES COMPILE_FLAGS "/GS-")
      ENDIF()
    ENDMACRO()

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMMON_CXX_FLAGS}")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DDEBUG")                     # enables assertions
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DTBB_USE_DEBUG")             # configures TBB in debug mode
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Ox")                         # enable full optimizations
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Oi")                         # inline intrinsic functions
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")        # generate debug information
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG")  # generate debug information

    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMMON_CXX_FLAGS}")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox")                       # enable full optimizations
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi")                       # inline intrinsic functions

#    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Gy")                       # package individual functions
#    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-vectorize")            # disable auto vectorization

    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${COMMON_CXX_FLAGS}")
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Ox")                      # enable full optimizations
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Oi")                      # inline intrinsic functions
    SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /DEBUG")        # generate debug information
    SET(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} /DEBUG")  # generate debug information
  ENDIF (MINGW)

  IF (MINGW)
    SET(SECURE_LINKER_FLAGS "")
    # It looks lld in llvm-mingw does not support these flags yet?
    #SET(SECURE_LINKER_FLAGS "${SECURE_LINKER_FLAGS} --nxcompat")
    # SET(SECURE_LINKER_FLAGS "${SECURE_LINKER_FLAGS} --dynamicbase")
  ELSE ()
    # cl.exe compatible mode?
    SET(SECURE_LINKER_FLAGS "")
    SET(SECURE_LINKER_FLAGS "${SECURE_LINKER_FLAGS} /NXCompat")    # compatible with data execution prevention (on by default)
    SET(SECURE_LINKER_FLAGS "${SECURE_LINKER_FLAGS} /DynamicBase") # random rebase of executable at load time
    IF (CMAKE_SIZEOF_VOID_P EQUAL 4)
      SET(SECURE_LINKER_FLAGS "${SECURE_LINKER_FLAGS} /SafeSEH")     # invoke known exception handlers (Win32 only, x64 exception handlers are safe by design)
    ENDIF()
  ENDIF()

  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SECURE_LINKER_FLAGS}")
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SECURE_LINKER_FLAGS}")

  IF (MSVC)
    INCLUDE(msvc_post)
  ENDIF (MSVC)
  
ELSE()

  OPTION(EMBREE_IGNORE_CMAKE_CXX_FLAGS "When enabled Embree ignores default CMAKE_CXX_FLAGS." OFF)
  OPTION(EMBREE_ADDRESS_SANITIZER "Enabled CLANG address sanitizer." OFF)

  IF (EMBREE_ADDRESS_SANITIZER)
    INCLUDE(CheckCXXCompilerFlag)

    set(_CLANG_ASAN_CXX_FLAGS "-fsanitize=address ")

    check_cxx_compiler_flag("-fsanitize-address-use-after-scope" HAS_SANITIZE_ADDRESS_USE_AFTER_SCOPE)
    if (HAS_SANITIZE_ADDRESS_USE_AFTER_SCOPE)
      string(APPEND _CLANG_ASAN_CXX_FLAGS " -fsanitize-address-use-after-scope ")
    endif()

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_CLANG_ASAN_CXX_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls")
  ENDIF()

  if (EMBREE_USE_PARENT_PROJECT_COMPILER_FLAGS)

    # Add -fsigned-char for safety
    # NOTE: arm's default interpretation of `char` is unsigned.
    #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsigned-char")

    #SET(CMAKE_CXX_FLAGS_DEBUG "")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsigned-char")

    #SET(CMAKE_CXX_FLAGS_RELEASE "")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsigned-char")

    #SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fsigned-char")

  ELSE (EMBREE_USE_PARENT_PROJECT_COMPILER_FLAGS)

    IF (EMBREE_IGNORE_CMAKE_CXX_FLAGS)
      SET(CMAKE_CXX_FLAGS "")
    ENDIF()

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsigned-char")               # treat 'char' as 'signed char'
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")                       # enables most warnings
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat -Wformat-security")  # enables string format vulnerability warnings
    IF (NOT APPLE)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE")                     # enables support for more secure position independent execution
    ENDIF()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")                       # generate position independent code suitable for shared libraries
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fPIC")                       # generate position independent code suitable for shared libraries
    IF (EMBREE_IOS)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")                # enables C++17 on iOS target
    ELSE(EMBREE_IOS)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")                # enables C++11 features
    ENDIF(EMBREE_IOS)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")         # makes all symbols hidden by default
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden") # makes all inline symbols hidden by default
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")        # disables strict aliasing rules
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-tree-vectorize")         # disable auto vectorizer
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2")         # perform extra security checks for some standard library calls
    IF (EMBREE_STACK_PROTECTOR)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")           # protects against return address overrides
    ENDIF()

    SET(CMAKE_CXX_FLAGS_DEBUG "")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsigned-char")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")              # generate debug information
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")         # enable assertions
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTBB_USE_DEBUG") # configure TBB in debug mode
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3")             # enable full optimizations

    SET(CMAKE_CXX_FLAGS_RELEASE "")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsigned-char")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")     # disable assertions
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")          # enable full optimizations

    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fsigned-char")
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g")              # generate debug information
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DNDEBUG")        # disable assertions
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3")             # enable full optimizations

    # pthread related code cannot be linked without lld when cross-compiling.
    if (EMBREE_USE_LLD)
      MESSAGE(STATUS "Use LLD")

      if (DEFINED ENV{LLD_PATH})
        set(LLD_PATH $ENV{LLD_PATH})
      else ()
        set(LLD_PATH "lld")
      endif ()
      
      SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=${LLD_PATH}")

    endif (EMBREE_USE_LLD)
      
  endif (EMBREE_USE_PARENT_PROJECT_COMPILER_FLAGS)

  MACRO(DISABLE_STACK_PROTECTOR_FOR_FILE file)
    IF (EMBREE_STACK_PROTECTOR)
      SET_SOURCE_FILES_PROPERTIES(${file} PROPERTIES COMPILE_FLAGS "-fno-stack-protector")
    ENDIF()
  ENDMACRO()

  IF (APPLE)
    IF (NOT IOS)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.7")   # makes sure code runs on older MacOSX versions
    ENDIF()
	  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")             # link against libc++ which supports C++11 features
  ELSE(APPLE)
    IF (NOT EMBREE_ADDRESS_SANITIZER) # for address sanitizer this causes link errors
      SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined") # issues link error for undefined symbols in shared library
      SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -z noexecstack")     # we do not need an executable stack
      SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -z relro -z now")    # re-arranges data sections to increase security
      SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z noexecstack")           # we do not need an executable stack
      SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z relro -z now")          # re-arranges data sections to increase security
      SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")                     # enables position independent execution for executable
    ENDIF()
  ENDIF(APPLE)

  IF (CMAKE_CROSSCOMPILING)
    # Assume cross-compiling for aarch64 linux
    # Need to add `-pthread`(It looks find_package(Threads) seems not to set `-pthread` when cross-compiling)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
  ENDIF ()

ENDIF()
