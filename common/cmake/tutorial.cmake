## Copyright 2009-2021 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

# additional parameters (beyond the name) are treated as additional dependencies
# if ADDITIONAL_LIBRARIES is set these will be included during linking

MACRO (ADD_TUTORIAL TUTORIAL_NAME)
  IF (MINGW)
    SET(TUTORIAL_EMBREE_RC ../../kernels/embree.mrc)
  ELSE()
    SET(TUTORIAL_EMBREE_RC ../../kernels/embree.rc)
  ENDIF()
  ADD_EXECUTABLE(${TUTORIAL_NAME} ${TUTORIAL_EMBREE_RC} ${TUTORIAL_NAME}.cpp ${TUTORIAL_NAME}_device.cpp ${ARGN})
  TARGET_LINK_LIBRARIES(${TUTORIAL_NAME} embree image tutorial noise ${ADDITIONAL_LIBRARIES})
  SET_PROPERTY(TARGET ${TUTORIAL_NAME} PROPERTY FOLDER tutorials/single)
  SET_PROPERTY(TARGET ${TUTORIAL_NAME} APPEND PROPERTY COMPILE_FLAGS " ${FLAGS_LOWEST}")
  INSTALL(TARGETS ${TUTORIAL_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT examples)
  SIGN_TARGET(${TUTORIAL_NAME})
ENDMACRO ()

MACRO (ADD_TUTORIAL_ISPC TUTORIAL_NAME)
  IF (MINGW)
    SET(TUTORIAL_EMBREE_RC ../../kernels/embree.mrc)
  ELSE()
    SET(TUTORIAL_EMBREE_RC ../../kernels/embree.rc)
  ENDIF()
  IF (EMBREE_ISPC_SUPPORT)
    ADD_EMBREE_ISPC_EXECUTABLE(${TUTORIAL_NAME}_ispc ${TUTORIAL_EMBREE_RC} ${TUTORIAL_NAME}.cpp ${TUTORIAL_NAME}_device.ispc ${ARGN})
    TARGET_LINK_LIBRARIES(${TUTORIAL_NAME}_ispc embree image tutorial_ispc noise noise_ispc)
    SET_PROPERTY(TARGET ${TUTORIAL_NAME}_ispc PROPERTY FOLDER tutorials/ispc)
    SET_PROPERTY(TARGET ${TUTORIAL_NAME}_ispc APPEND PROPERTY COMPILE_FLAGS " ${FLAGS_LOWEST}")
    INSTALL(TARGETS ${TUTORIAL_NAME}_ispc DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT examples)
    SIGN_TARGET(${TUTORIAL_NAME}_ispc)
  ENDIF()
ENDMACRO ()
