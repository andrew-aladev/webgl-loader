cmake_minimum_required (VERSION 2.8.9)
project (webgl-loader-meta)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(SEND_ERROR "In-source builds are not allowed! mkdir build, cd build, cmake ..")
endif()

set (CMAKE_BUILD_TYPE Debug)
set (CMAKE_VERBOSE_MAKEFILE on)

set (CMAKE_CXX_FLAGS_DEBUG "-std=c++0x -O2 -g")
set (CMAKE_CXX_FLAGS_RELEASE "-std=c++0x -O2")

add_subdirectory("${PROJECT_SOURCE_DIR}/src")
add_subdirectory("${PROJECT_SOURCE_DIR}/utils")