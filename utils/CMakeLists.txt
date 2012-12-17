project (webgl-utils)

include_directories (
	${PROJECT_SOURCE_DIR}/../include
)

add_subdirectory("${PROJECT_SOURCE_DIR}/all-codepoints")
add_subdirectory("${PROJECT_SOURCE_DIR}/bounds-test")
add_subdirectory("${PROJECT_SOURCE_DIR}/compress-test")
add_subdirectory("${PROJECT_SOURCE_DIR}/face-list-test")
add_subdirectory("${PROJECT_SOURCE_DIR}/good-codepoints")
add_subdirectory("${PROJECT_SOURCE_DIR}/hex-sanity")
add_subdirectory("${PROJECT_SOURCE_DIR}/histo")
add_subdirectory("${PROJECT_SOURCE_DIR}/json-test")
add_subdirectory("${PROJECT_SOURCE_DIR}/obj2utf8")
add_subdirectory("${PROJECT_SOURCE_DIR}/obj2utf8x")
# TODO: fix "class WavefrontObjFile has no member named CreateDrawMeshes"
# add_subdirectory("${PROJECT_SOURCE_DIR}/objanalyze")
add_subdirectory("${PROJECT_SOURCE_DIR}/objcompress")
add_subdirectory("${PROJECT_SOURCE_DIR}/stream-test")
add_subdirectory("${PROJECT_SOURCE_DIR}/wavefront-obj-file-test")