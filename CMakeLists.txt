cmake_minimum_required (VERSION 2.8.9)
project (webgl-loader)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(SEND_ERROR "In-source builds are not allowed! mkdir build, cd build, cmake ..")
endif()

set (CMAKE_BUILD_TYPE Debug)
set (CMAKE_VERBOSE_MAKEFILE on)

set (CMAKE_CXX_FLAGS_DEBUG "-O2 -g")
set (CMAKE_CXX_FLAGS_RELEASE "-O2")

include_directories (
	${PROJECT_SOURCE_DIR}/include
)
add_subdirectory("${PROJECT_SOURCE_DIR}/src/all-codepoints")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/bounds-test")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/compress-test")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/face-list-test")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/good-codepoints")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/hex-sanity")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/histo")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/json-test")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/obj2utf8")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/obj2utf8x")
# TODO: fix "class WavefrontObjFile has no member named CreateDrawMeshes"
# add_subdirectory("${PROJECT_SOURCE_DIR}/src/objanalyze")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/objcompress")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/stream-test")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/wavefront-obj-file-test")