## Copyright 2009-2020 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

ADD_LIBRARY(imgui STATIC
  imgui.cpp
  imgui_impl_glfw_gl2.cpp
  imgui_draw.cpp
)

TARGET_LINK_LIBRARIES(imgui PUBLIC ${OPENGL_LIBRARIES} glfw)
SET_PROPERTY(TARGET imgui PROPERTY FOLDER tutorials/common)
SET_PROPERTY(TARGET imgui APPEND PROPERTY COMPILE_FLAGS " ${FLAGS_LOWEST}")
