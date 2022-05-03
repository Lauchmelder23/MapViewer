cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE CPP_FILES 
	"src/*.c"
)

file(GLOB_RECURSE HPP_FILES 
	"include/*.h"
)

SET_SOURCE_FILES_PROPERTIES( ${CPP_FILES} PROPERTIES LANGUAGE CXX )

add_library(triangle STATIC 
	${CPP_FILES} ${HPP_FILES}
)

target_include_directories(triangle PUBLIC 
	include 
	${TINYXML2_INCLUDE_DIR}
)

target_compile_definitions(triangle PUBLIC NO_TIMER TRILIBRARY REDUCED DCT_ONLY ANSI_DECLARATORS)