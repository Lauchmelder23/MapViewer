cmake_minimum_required(VERSION 3.10)

find_package(tinyxml2 CONFIG REQUIRED)

file(GLOB_RECURSE CPP_FILES 
	"src/*.cpp"
)

file(GLOB_RECURSE HPP_FILES 
	"include/*.hpp"
)

get_target_property(TINYXML2_INCLUDE_DIR tinyxml2::tinyxml2 INTERFACE_INCLUDE_DIRECTORIES)

add_library(osmp STATIC 
	${CPP_FILES}
)

target_include_directories(osmp PUBLIC 
	include 
	${TINYXML2_INCLUDE_DIR}
)

target_link_libraries(osmp PRIVATE 
	tinyxml2::tinyxml2
)