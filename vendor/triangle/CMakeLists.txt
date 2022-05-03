cmake_minimum_required(VERSION 3.10)

SET_SOURCE_FILES_PROPERTIES( "src/triangle.c" PROPERTIES LANGUAGE CXX )

add_library(triangle STATIC 
	"src/triangle.c"
)

target_include_directories(triangle PUBLIC 
	${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_definitions(triangle PUBLIC NO_TIMER TRILIBRARY REDUCED DCT_ONLY ANSI_DECLARATORS)