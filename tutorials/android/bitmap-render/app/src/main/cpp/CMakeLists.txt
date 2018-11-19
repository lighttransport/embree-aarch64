cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wno-unused-function")

include_directories(${EMBREE_ANDROID_PATH}/include)
link_directories(${EMBREE_ANDROID_PATH}/lib)


add_library(embreerender SHARED
            embreerender.cc
            obj-loader.cc)

# Include libraries needed for plasma lib
target_link_libraries(embreerender
                      android
                      jnigraphics
                      embree3
                      log
                      m)
