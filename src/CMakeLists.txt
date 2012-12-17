project (webgl-loader)

include_directories (
	${PROJECT_SOURCE_DIR}/../include
)

add_library(
	${PROJECT_NAME} SHARED
	${PROJECT_SOURCE_DIR}/base.cpp
)