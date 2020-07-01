## Copyright 2009-2020 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

IF (NOT WIN32 OR EMBREE_EXTERNAL_GLFW)

  FIND_PACKAGE(glfw3 REQUIRED)
  SET(GLFW_LIBRARY ${GLFW3_LIBRARY})
  SET(GLFW_INCLUDE_DIRS ${GLFW3_INCLUDE_PATH})
ELSE()

  # detect and select Win32 or x64
  IF (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(ARCH x64)
  ELSE()
    SET(ARCH Win32)
  ENDIF()

  # detect Visual Studio version
  IF (MSVC12)
    SET(VCVER vc12)
  ELSE()
    SET(VCVER vc14)
  ENDIF()

  FIND_PATH(GLFW_INCLUDE_DIRS NAMES GLFW/glfw3.h PATHS common/glfw/include)
  FIND_LIBRARY(GLFW_LIBRARY glfw3 common/glfw/${ARCH}/${VCVER})
  IF (GLFW_INCLUDE_DIRS AND GLFW_LIBRARY)
    SET(GLFW_FOUND 1)
  ENDIF()
  SET(GLFW_STATIC_LIBRARIES ${GLFW_LIBRARY})
  MARK_AS_ADVANCED(
    GLFW_INCLUDE_DIR
    GLFW_LIBRARY
  )
ENDIF()

IF (GLFW_FOUND)
  FIND_PACKAGE(OpenGL)
  IF (NOT OPENGL_FOUND)
    MESSAGE(FATAL_ERROR "OpenGL not found! Please either install OpenGL or disable EMBREE_TUTORIALS.")
  ENDIF()

  INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS})
ENDIF (GLFW_FOUND)


ADD_SUBDIRECTORY(common)

ADD_SUBDIRECTORY(verify)

IF (GLFW_FOUND)
  ADD_SUBDIRECTORY(triangle_geometry)
  ADD_SUBDIRECTORY(dynamic_scene)
  ADD_SUBDIRECTORY(user_geometry)
  ADD_SUBDIRECTORY(viewer)
  ADD_SUBDIRECTORY(instanced_geometry)

  if ("${EMBREE_MAX_INSTANCE_LEVEL_COUNT}" GREATER "1")
    ADD_SUBDIRECTORY(multi_instanced_geometry)
  endif()

  ADD_SUBDIRECTORY(intersection_filter)
  ADD_SUBDIRECTORY(pathtracer)
  ADD_SUBDIRECTORY(hair_geometry)
  ADD_SUBDIRECTORY(subdivision_geometry)
  ADD_SUBDIRECTORY(displacement_geometry)
  ADD_SUBDIRECTORY(grid_geometry)
  ADD_SUBDIRECTORY(lazy_geometry)
  ADD_SUBDIRECTORY(bvh_access)
  ADD_SUBDIRECTORY(minimal)
  ADD_SUBDIRECTORY(motion_blur_geometry)
  ADD_SUBDIRECTORY(interpolation)
  ADD_SUBDIRECTORY(convert)
  ADD_SUBDIRECTORY(curve_geometry)
  ADD_SUBDIRECTORY(point_geometry)
  ADD_SUBDIRECTORY(collide)
  ADD_SUBDIRECTORY(closest_point)
  ADD_SUBDIRECTORY(voronoi)
  ADD_SUBDIRECTORY(next_hit)
  ADD_SUBDIRECTORY(multiscene_geometry)
  ADD_SUBDIRECTORY(quaternion_motion_blur)
  IF (EMBREE_RAY_PACKETS)
    ADD_SUBDIRECTORY(viewer_stream)
    ADD_SUBDIRECTORY(viewer_anim)
  ENDIF()
ENDIF()

ADD_SUBDIRECTORY(bvh_builder)
ADD_SUBDIRECTORY(buildbench)
