## Copyright 2009-2020 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

IF (MINGW)
  ADD_EXECUTABLE(minimal ../../kernels/embree.mrc minimal.cpp)
ELSE()
  ADD_EXECUTABLE(minimal ../../kernels/embree.rc minimal.cpp)
ENDIF()
TARGET_LINK_LIBRARIES(minimal embree)
TARGET_INCLUDE_DIRECTORIES(minimal PRIVATE ${PROJECT_SOURCE_DIR}/include)
SET_PROPERTY(TARGET minimal PROPERTY FOLDER tutorials/single)
SET_PROPERTY(TARGET minimal APPEND PROPERTY COMPILE_FLAGS " ${FLAGS_LOWEST}")
INSTALL(TARGETS minimal DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT examples)
SIGN_TARGET(minimal)
