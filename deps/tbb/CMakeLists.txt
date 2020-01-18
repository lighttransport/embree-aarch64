# CMakeLists.txt based on https://github.com/wjakob/tbb
#
# We just want to build TBB as a static lib and built tbb using `add_subdirectory`,
# So added some modification
#
#cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

#if (POLICY CMP0048)
#  # cmake warns if loaded from a min-3.0-required parent dir, so silence the warning:
#  cmake_policy(SET CMP0048 NEW)
#endif()

project (tbb CXX)

find_package(Threads REQUIRED)

#include_directories(include src src/rml/include ${CMAKE_CURRENT_BINARY_DIR})

set(TBB_BUILD_STATIC ON)
set(TBB_BUILD_TBBMALLOC ON)

#option(TBB_BUILD_SHARED          "Build TBB shared library" ON)
#option(TBB_BUILD_STATIC          "Build TBB static library" ON)
#option(TBB_BUILD_TBBMALLOC       "Build TBB malloc library" ON)
#option(TBB_BUILD_TBBMALLOC_PROXY "Build TBB malloc proxy library" ON)
#option(TBB_BUILD_TESTS           "Build TBB tests and enable testing infrastructure" ON)
#option(TBB_CI_BUILD              "Is this a continuous integration build?" OFF)
#option(TBB_NO_DATE               "Do not save the configure date in the version string" OFF)

set(TBB_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../third_party/tbb)

if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
endif()

set(tbb_src 

  ${TBB_SOURCE_DIR}/src/tbb/arena.cpp
  ${TBB_SOURCE_DIR}/src/tbb/cache_aligned_allocator.cpp
  ${TBB_SOURCE_DIR}/src/tbb/concurrent_hash_map.cpp
  ${TBB_SOURCE_DIR}/src/tbb/concurrent_monitor.cpp
  ${TBB_SOURCE_DIR}/src/tbb/concurrent_queue.cpp
  ${TBB_SOURCE_DIR}/src/tbb/concurrent_vector.cpp
  ${TBB_SOURCE_DIR}/src/tbb/condition_variable.cpp
  ${TBB_SOURCE_DIR}/src/tbb/critical_section.cpp
  ${TBB_SOURCE_DIR}/src/tbb/dynamic_link.cpp
  ${TBB_SOURCE_DIR}/src/tbb/governor.cpp
  ${TBB_SOURCE_DIR}/src/tbb/itt_notify.cpp
  ${TBB_SOURCE_DIR}/src/tbb/market.cpp
  ${TBB_SOURCE_DIR}/src/tbb/observer_proxy.cpp
  ${TBB_SOURCE_DIR}/src/tbb/pipeline.cpp
  ${TBB_SOURCE_DIR}/src/tbb/private_server.cpp
  ${TBB_SOURCE_DIR}/src/tbb/queuing_mutex.cpp
  ${TBB_SOURCE_DIR}/src/tbb/queuing_rw_mutex.cpp
  ${TBB_SOURCE_DIR}/src/tbb/reader_writer_lock.cpp
  ${TBB_SOURCE_DIR}/src/tbb/scheduler.cpp
  ${TBB_SOURCE_DIR}/src/tbb/semaphore.cpp
  ${TBB_SOURCE_DIR}/src/tbb/spin_mutex.cpp
  ${TBB_SOURCE_DIR}/src/tbb/spin_rw_mutex.cpp
  ${TBB_SOURCE_DIR}/src/tbb/task.cpp
  ${TBB_SOURCE_DIR}/src/tbb/task_group_context.cpp
  ${TBB_SOURCE_DIR}/src/tbb/tbbbind.cpp
  ${TBB_SOURCE_DIR}/src/tbb/tbb_main.cpp
  ${TBB_SOURCE_DIR}/src/tbb/tbb_misc.cpp
  ${TBB_SOURCE_DIR}/src/tbb/tbb_misc_ex.cpp
  ${TBB_SOURCE_DIR}/src/tbb/tbb_statistics.cpp
  ${TBB_SOURCE_DIR}/src/tbb/tbb_thread.cpp
  ${TBB_SOURCE_DIR}/src/tbb/x86_rtm_rw_mutex.cpp

  # deprecated
  #${TBB_SOURCE_DIR}/src/tbb/recursive_mutex.cpp
  #${TBB_SOURCE_DIR}/src/tbb/mutex.cpp
)


#file(GLOB tbb_src "${TBB_SOURCE_DIR}/src/tbb/*.cpp" "${TBB_SOURCE_DIR}/src/old/*.cpp")

list(APPEND tbb_src ${PROJECT_SOURCE_DIR}/../../third_party/tbb/src/rml/client/rml_tbb.cpp)

#file(GLOB to_remove "${TBB_SOURCE_DIR}/src/old/test*.cpp")

#list(REMOVE_ITEM tbb_src ${to_remove})

set(tbbmalloc_static_src
  ${TBB_SOURCE_DIR}/src/tbbmalloc/backend.cpp
  ${TBB_SOURCE_DIR}/src/tbbmalloc/large_objects.cpp
  ${TBB_SOURCE_DIR}/src/tbbmalloc/backref.cpp
  ${TBB_SOURCE_DIR}/src/tbbmalloc/tbbmalloc.cpp
  #${TBB_SOURCE_DIR}/src/tbbmalloc/frontend.cpp
  ${TBB_SOURCE_DIR}/src/tbb/itt_notify.cpp)

set(tbbmalloc_src ${tbbmalloc_static_src})

set(tbbmalloc_proxy_src
  ${TBB_SOURCE_DIR}/src/tbbmalloc/proxy.cpp
  ${TBB_SOURCE_DIR}/src/tbbmalloc/tbb_function_replacement.cpp)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(i386|x86_64)")
  if (NOT APPLE AND NOT MINGW)
    add_definitions(-DDO_ITT_NOTIFY)
  endif()
endif()

if (APPLE)
  # Disable annoying "has no symbols" warnings
  set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
  set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif()

macro(CHECK_CXX_COMPILER_AND_LINKER_FLAGS _RESULT _CXX_FLAGS _LINKER_FLAGS)
  set(CMAKE_REQUIRED_FLAGS ${_CXX_FLAGS})
  set(CMAKE_REQUIRED_LIBRARIES ${_LINKER_FLAGS})
  set(CMAKE_REQUIRED_QUIET TRUE)
  #check_cxx_source_runs("#include <iostream>\nint main(int argc, char **argv) { std::cout << \"test\"; return 0; }" ${_RESULT})
  set(CMAKE_REQUIRED_FLAGS "")
  set(CMAKE_REQUIRED_LIBRARIES "")
endmacro()

if (UNIX)
  #add_definitions (-DUSE_PTHREAD)

  #check_cxx_compiler_flag ("-mrtm -Werror" SUPPORTS_MRTM)
  #if (SUPPORTS_MRTM)
  #  set (CMAKE_CXX_FLAGS "-mrtm ${CMAKE_CXX_FLAGS}")
  #endif ()

elseif(WIN32)
  if (MSVC)
    enable_language(ASM_MASM)
    set(CMAKE_CXX_FLAGS "/GS- /Zc:wchar_t /Zc:forScope /DUSE_WINTHREAD ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "/D_CRT_SECURE_NO_DEPRECATE /D_WIN32_WINNT=0x0600 ${CMAKE_CXX_FLAGS}")
    #check_cxx_compiler_flag ("/volatile:iso" SUPPORTS_VOLATILE_FLAG)
    if (SUPPORTS_VOLATILE_FLAG)
      set(CMAKE_CXX_FLAGS "/volatile:iso ${CMAKE_CXX_FLAGS}")
    endif ()
    set(CMAKE_CXX_FLAGS "/wd4267 /wd4800 /wd4146 /wd4244 /wd4577 /wd4018 ${CMAKE_CXX_FLAGS}")
    if (NOT CMAKE_SIZEOF_VOID_P)
       message(FATAL_ERROR "'CMAKE_SIZEOF_VOID_P' is undefined. Please delete your build directory and rerun CMake again!")
    endif()

    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
      list(APPEND tbb_src src/tbb/intel64-masm/atomic_support.asm
        src/tbb/intel64-masm/itsx.asm src/tbb/intel64-masm/intel64_misc.asm)
      list(APPEND tbbmalloc_src src/tbb/intel64-masm/atomic_support.asm)
      set(CMAKE_ASM_MASM_FLAGS "/DEM64T=1 ${CMAKE_ASM_MASM_FLAGS}")
    else()
      list(APPEND tbb_src src/tbb/ia32-masm/atomic_support.asm
        src/tbb/ia32-masm/itsx.asm src/tbb/ia32-masm/lock_byte.asm)
      # Enable SAFESEH feature for assembly (x86 builds only).
      set(CMAKE_ASM_MASM_FLAGS "/safeseh ${CMAKE_ASM_MASM_FLAGS}")
    endif()
  elseif (MINGW)
    add_definitions(-DUSE_WINTHREAD)
    add_definitions(-D_WIN32_WINNT=0x0502)
    set(CMAKE_CXX_FLAGS "-mthreads ${CMAKE_CXX_FLAGS}")
  endif ()
endif()

#if (MSVC)
#  set(ENABLE_RTTI "/EHsc /GR ")
#  set(DISABLE_RTTI "/EHs- /GR- ")
#elseif (UNIX)
#  set(ENABLE_RTTI "-frtti -fexceptions ")
#  set(DISABLE_RTTI "-fno-rtti -fno-exceptions ")
#endif ()

###--------
##   - Added TBB_USE_GLIBCXX_VERSION macro to specify the version of GNU
##     libstdc++ when it cannot be properly recognized, e.g. when used
##     with Clang on Linux* OS. Inspired by a contribution from David A.
#if (NOT TBB_USE_GLIBCXX_VERSION AND UNIX AND NOT APPLE)
#  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
#    # using Clang
#    string(REPLACE "." "0" TBB_USE_GLIBCXX_VERSION ${CMAKE_CXX_COMPILER_VERSION})
#  endif()
#endif()
#
#if (TBB_USE_GLIBCXX_VERSION)
#   add_definitions(-DTBB_USE_GLIBCXX_VERSION=${TBB_USE_GLIBCXX_VERSION})
#endif()
#
###-------

#if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
#   check_cxx_compiler_flag ("-flifetime-dse=1" SUPPORTS_FLIFETIME)
#   if (SUPPORTS_FLIFETIME)
#     add_definitions(-flifetime-dse=1)
#   endif()
#endif()

# Linker export definitions
if (APPLE)
  set (ARCH_PREFIX "mac")
elseif(WIN32)
  set (ARCH_PREFIX "win")
else()
  set (ARCH_PREFIX "lin")
endif()

#if (CMAKE_SIZEOF_VOID_P EQUAL 8)
#  set(ARCH_PREFIX "${ARCH_PREFIX}64")
#else()
#  set(ARCH_PREFIX "${ARCH_PREFIX}32")
#endif()
#
#if (MINGW)
#  set (ARCH_PREFIX "${ARCH_PREFIX}-gcc")
#  # there's no win32-gcc-tbb-export.def, use lin32-tbb-export.def
#  execute_process (COMMAND ${CMAKE_COMMAND} -E copy ${TBB_SOURCE_DIR}/src/tbb/lin32-tbb-export.def ${TBB_SOURCE_DIR}/src/tbb/win32-gcc-tbb-export.def)
#endif()
#
#if (MSVC)
#  add_custom_command(OUTPUT tbb.def
#    COMMAND ${CMAKE_CXX_COMPILER} /TC /EP ${TBB_SOURCE_DIR}/src/tbb/${ARCH_PREFIX}-tbb-export.def  -I ${TBB_SOURCE_DIR}/include > tbb.def
#    MAIN_DEPENDENCY ${TBB_SOURCE_DIR}/src/tbb/${ARCH_PREFIX}-tbb-export.def
#    COMMENT "Preprocessing tbb.def"
#  )
#
#  add_custom_command(OUTPUT tbbmalloc.def
#    COMMAND ${CMAKE_CXX_COMPILER} /TC /EP ${TBB_SOURCE_DIR}/src/tbbmalloc/${ARCH_PREFIX}-tbbmalloc-export.def  -I ${TBB_SOURCE_DIR}/include >   tbbmalloc.def
#    MAIN_DEPENDENCY ${TBB_SOURCE_DIR}/src/tbbmalloc/${ARCH_PREFIX}-tbbmalloc-export.def
#    COMMENT "Preprocessing tbbmalloc.def"
#  )
#else()
#  add_custom_command(OUTPUT tbb.def
#    COMMAND ${CMAKE_CXX_COMPILER} -xc++ -E ${TBB_SOURCE_DIR}/src/tbb/${ARCH_PREFIX}-tbb-export.def  -I ${TBB_SOURCE_DIR}/include -o tbb.def
#    MAIN_DEPENDENCY ${TBB_SOURCE_DIR}/src/tbb/${ARCH_PREFIX}-tbb-export.def
#    COMMENT "Preprocessing tbb.def"
#  )
#
#  add_custom_command(OUTPUT tbbmalloc.def
#    COMMAND ${CMAKE_CXX_COMPILER} -xc++ -E ${TBB_SOURCE_DIR}/src/tbbmalloc/${ARCH_PREFIX}-tbbmalloc-export.def  -I ${TBB_SOURCE_DIR}/include -o   tbbmalloc.def
#    MAIN_DEPENDENCY ${TBB_SOURCE_DIR}/src/tbbmalloc/${ARCH_PREFIX}-tbbmalloc-export.def
#    COMMENT "Preprocessing tbbmalloc.def"
#  )
#endif()

#add_custom_target(tbb_def_files DEPENDS tbb.def tbbmalloc.def)

# TBB library
if (TBB_BUILD_STATIC)
  add_library(tbb_static STATIC ${tbb_src})
  target_include_directories(tbb_static PUBLIC "${TBB_SOURCE_DIR}/include")
  target_include_directories(tbb_static PRIVATE "${TBB_SOURCE_DIR}/src")
  # for "version_string.ver"
  target_include_directories(tbb_static PRIVATE "${PROJECT_SOURCE_DIR}")
  set_property(TARGET tbb_static APPEND PROPERTY COMPILE_DEFINITIONS "__TBB_BUILD=1")
  set_property(TARGET tbb_static APPEND PROPERTY COMPILE_DEFINITIONS "__TBB_DYNAMIC_LOAD_ENABLED=0")
  set_property(TARGET tbb_static APPEND PROPERTY COMPILE_DEFINITIONS "__TBB_SOURCE_DIRECTLY_INCLUDED=1")
  if (UNIX) 
    set_property(TARGET tbb_static APPEND PROPERTY COMPILE_DEFINITIONS "USE_PTHREAD")
  endif()
  #set_property(TARGET tbb_static APPEND_STRING PROPERTY COMPILE_FLAGS ${ENABLE_RTTI})
  #install(TARGETS tbb_static ARCHIVE DESTINATION ${TBB_INSTALL_ARCHIVE_DIR})
  if (MSVC)
    target_compile_definitions(tbb_static PUBLIC __TBB_NO_IMPLICIT_LINKAGE=1)
  endif()

  if (UNIX AND NOT APPLE)
    target_link_libraries(tbb_static PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
  endif()
endif()

if (TBB_BUILD_SHARED)
  add_library(tbb SHARED ${tbb_src})
  target_include_directories(tbb INTERFACE "${TBB_SOURCE_DIR}/include")
  set_property(TARGET tbb APPEND PROPERTY COMPILE_DEFINITIONS "__TBB_BUILD=1")
  #set_property(TARGET tbb APPEND_STRING PROPERTY COMPILE_FLAGS ${ENABLE_RTTI})
  add_dependencies(tbb tbb_def_files)

  if (APPLE)
    set_property(TARGET tbb APPEND PROPERTY LINK_FLAGS "-Wl,-exported_symbols_list,\"${CMAKE_CURRENT_BINARY_DIR}/tbb.def\"")
  elseif (MSVC)
    set_property(TARGET tbb APPEND PROPERTY LINK_FLAGS "/DEF:\"${CMAKE_CURRENT_BINARY_DIR}/tbb.def\"")
  else ()
    set_property(TARGET tbb APPEND PROPERTY LINK_FLAGS "-Wl,-version-script,\"${CMAKE_CURRENT_BINARY_DIR}/tbb.def\"")
  endif()

  #install(TARGETS tbb
  #        LIBRARY DESTINATION ${TBB_INSTALL_LIBRARY_DIR}
  #        ARCHIVE DESTINATION ${TBB_INSTALL_ARCHIVE_DIR}
  #        RUNTIME DESTINATION ${TBB_INSTALL_RUNTIME_DIR})
  #if (MSVC)
  #  install(FILES $<TARGET_PDB_FILE:tbb> DESTINATION ${TBB_INSTALL_RUNTIME_DIR} OPTIONAL)
  #endif()
  if (UNIX AND NOT APPLE)
    target_link_libraries(tbb PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
  endif()
  if (MSVC)
    target_compile_definitions(tbb PUBLIC __TBB_NO_IMPLICIT_LINKAGE=1)
  endif()
endif()


if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # Quench a warning on GCC
  set_source_files_properties(${TBB_SOURCE_DIR}/src/tbb/governor.cpp COMPILE_FLAGS "-Wno-missing-field-initializers ")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # Quench a warning on Clang
  set_source_files_properties(${TBB_SOURCE_DIR}/src/tbb/itt_notify.cpp COMPILE_FLAGS "-Wno-varargs ")
elseif(MSVC)
  # Quench a warning on MSVC
  set_source_files_properties(${TBB_SOURCE_DIR}/src/tbb/scheduler.cpp COMPILE_FLAGS "/wd4458 ")
endif()

if(TBB_BUILD_TBBMALLOC)
  # TBB malloc library
  if (TBB_BUILD_STATIC)
    add_library(tbbmalloc_static STATIC ${tbbmalloc_static_src})
    target_include_directories(tbbmalloc_static PRIVATE "${TBB_SOURCE_DIR}/src")
    # for "version_string.ver"
    target_include_directories(tbbmalloc_static PRIVATE "${PROJECT_SOURCE_DIR}")
    set_property(TARGET tbbmalloc_static APPEND PROPERTY COMPILE_DEFINITIONS "__TBBMALLOC_BUILD=1")
    set_property(TARGET tbbmalloc_static APPEND PROPERTY COMPILE_DEFINITIONS "__TBB_DYNAMIC_LOAD_ENABLED=0")
    set_property(TARGET tbbmalloc_static APPEND PROPERTY COMPILE_DEFINITIONS "__TBB_SOURCE_DIRECTLY_INCLUDED=1")
    set_property(TARGET tbbmalloc_static APPEND_STRING PROPERTY COMPILE_FLAGS ${DISABLE_RTTI})
    if (MSVC)
      target_compile_definitions(tbbmalloc_static PUBLIC __TBB_NO_IMPLICIT_LINKAGE=1 __TBBMALLOC_NO_IMPLICIT_LINKAGE=1)
    endif()
    #install(TARGETS tbbmalloc_static ARCHIVE DESTINATION ${TBB_INSTALL_ARCHIVE_DIR})
  endif()

  if (TBB_BUILD_SHARED)
    add_library(tbbmalloc SHARED ${tbbmalloc_src})
    set_property(TARGET tbbmalloc APPEND PROPERTY COMPILE_DEFINITIONS "__TBBMALLOC_BUILD=1")
    set_property(TARGET tbbmalloc APPEND_STRING PROPERTY COMPILE_FLAGS ${DISABLE_RTTI})
    add_dependencies(tbbmalloc tbb_def_files)
    if (APPLE)
      set_property(TARGET tbbmalloc APPEND PROPERTY LINK_FLAGS "-Wl,-exported_symbols_list,\"${CMAKE_CURRENT_BINARY_DIR}/tbbmalloc.def\"")
    elseif (MSVC)
      set_property(TARGET tbbmalloc APPEND PROPERTY LINK_FLAGS "/DEF:\"${CMAKE_CURRENT_BINARY_DIR}/tbbmalloc.def\"")
    else ()
      set_property(TARGET tbbmalloc APPEND PROPERTY LINK_FLAGS "-Wl,-version-script,\"${CMAKE_CURRENT_BINARY_DIR}/tbbmalloc.def\"")
    endif()
    if (MSVC)
      target_compile_definitions(tbbmalloc PUBLIC __TBB_NO_IMPLICIT_LINKAGE=1 __TBBMALLOC_NO_IMPLICIT_LINKAGE=1)
    endif()
    #install(TARGETS tbbmalloc
    #        LIBRARY DESTINATION ${TBB_INSTALL_LIBRARY_DIR}
    #        ARCHIVE DESTINATION ${TBB_INSTALL_ARCHIVE_DIR}
    #        RUNTIME DESTINATION ${TBB_INSTALL_RUNTIME_DIR})
    #if (MSVC)
    #  install(FILES $<TARGET_PDB_FILE:tbbmalloc> DESTINATION ${TBB_INSTALL_RUNTIME_DIR} OPTIONAL)
    #endif()
    if (UNIX AND NOT APPLE)
      target_link_libraries(tbbmalloc PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
    endif()
  endif()
endif()

if(TBB_BUILD_TBBMALLOC_PROXY)
  # TBB malloc proxy library
  if (TBB_BUILD_STATIC)
    add_library(tbbmalloc_proxy_static STATIC ${tbbmalloc_proxy_src})
    set_property(TARGET tbbmalloc_proxy_static APPEND PROPERTY COMPILE_DEFINITIONS "__TBBMALLOC_BUILD=1")
    set_property(TARGET tbbmalloc_proxy_static APPEND PROPERTY COMPILE_DEFINITIONS "__TBB_DYNAMIC_LOAD_ENABLED=0")
    set_property(TARGET tbbmalloc_proxy_static APPEND PROPERTY COMPILE_DEFINITIONS "__TBB_SOURCE_DIRECTLY_INCLUDED=1")
    set_property(TARGET tbbmalloc_proxy_static APPEND_STRING PROPERTY COMPILE_FLAGS ${DISABLE_RTTI})
    #install(TARGETS tbbmalloc_proxy_static ARCHIVE DESTINATION ${TBB_INSTALL_ARCHIVE_DIR})
  endif()

  if (TBB_BUILD_SHARED)
    add_library(tbbmalloc_proxy SHARED ${tbbmalloc_proxy_src})
    set_property(TARGET tbbmalloc_proxy APPEND PROPERTY COMPILE_DEFINITIONS "__TBBMALLOC_BUILD=1")
    set_property(TARGET tbbmalloc_proxy APPEND_STRING PROPERTY COMPILE_FLAGS ${DISABLE_RTTI})
    target_link_libraries(tbbmalloc_proxy PUBLIC tbbmalloc)
    #install(TARGETS tbbmalloc_proxy
    #        LIBRARY DESTINATION ${TBB_INSTALL_LIBRARY_DIR}
    #        ARCHIVE DESTINATION ${TBB_INSTALL_ARCHIVE_DIR}
    #        RUNTIME DESTINATION ${TBB_INSTALL_RUNTIME_DIR})
    #if (MSVC)
    #  install(FILES $<TARGET_PDB_FILE:tbbmalloc_proxy> DESTINATION ${TBB_INSTALL_RUNTIME_DIR} OPTIONAL)
    #endif()
    if (UNIX AND NOT APPLE)
      target_link_libraries(tbbmalloc_proxy PUBLIC Threads::Threasd ${CMAKE_DL_LIBS})
    endif()
  endif()
endif()

set(TBB_LIBRARY tbb_static)
set(TBB_LIBRARY_MALLOC tbbmalloc_static)

# Notify parent CMake can look-up these variables.
# TODO: Use find_package?
set(TBB_LIBRARIES ${TBB_LIBRARY} ${TBB_LIBRARY_MALLOC} PARENT_SCOPE)
set(TBB_INCLUDE_DIRS ${TBB_SOURCE_DIR}/include PARENT_SCOPE)

mark_as_advanced(TBB_INCLUDE_DIRS)
mark_as_advanced(TBB_LIBRARIES)

#install(DIRECTORY include/tbb DESTINATION ${TBB_INSTALL_INCLUDE_DIR})

# version_string.ver
#if (UNIX AND NOT TBB_NO_DATE)
#  execute_process (COMMAND date "+%a, %d %b %Y %H:%M:%S %z"
#                   OUTPUT_VARIABLE _configure_date
#                   OUTPUT_STRIP_TRAILING_WHITESPACE)
#elseif (WIN32 AND NOT TBB_NO_DATE)
#  execute_process (COMMAND cmd " /C date /T"
#                   OUTPUT_VARIABLE _configure_date
#                   OUTPUT_STRIP_TRAILING_WHITESPACE)
#else ()
#  set (_configure_date "Unknown")
#endif()
#set (TBB_CONFIG_DATE "${_configure_date}" CACHE STRING "First time that TBB was configured")
#set (_configure_date "${TBB_CONFIG_DATE}")
#include_directories (${CMAKE_BINARY_DIR})
#configure_file (../../third_party/tbb/build/version_string.ver.in version_string.ver @ONLY)
