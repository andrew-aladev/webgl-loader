project (webgl-loader)

include_directories (
	${PROJECT_SOURCE_DIR}/../include
)

add_library(
	${PROJECT_NAME} SHARED
	${PROJECT_SOURCE_DIR}/base.cpp
	${PROJECT_SOURCE_DIR}/bounds.cpp
	${PROJECT_SOURCE_DIR}/compress.cpp
	${PROJECT_SOURCE_DIR}/json.cpp
	${PROJECT_SOURCE_DIR}/mesh.cpp
	${PROJECT_SOURCE_DIR}/optimize.cpp
	${PROJECT_SOURCE_DIR}/utf8.cpp
)