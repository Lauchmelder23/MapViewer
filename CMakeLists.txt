# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("MapViewer")

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2-GFX CONFIG REQUIRED)

# Include sub-projects.
add_subdirectory ("lib/osmparser")
add_subdirectory ("lib/triangle")

file(GLOB_RECURSE SOURCE_FILES
	"src/*.cpp"
)

file(GLOB_RECURSE INCLUDE_FILES
	"include/*.hpp"
)

add_executable(mapviewer 
	${INCLUDE_FILES} ${SOURCE_FILES}
)

target_include_directories(mapviewer PRIVATE 
	osmp 
	triangle
	SDL2::SDL2 SDL2::SDL2_gfx
)

target_link_libraries(mapviewer PRIVATE 
	osmp
	triangle
	SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_gfx
)

add_custom_command(TARGET mapviewer POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/res/map.osm $<TARGET_FILE_DIR:mapviewer>
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/res/bigmap.osm $<TARGET_FILE_DIR:mapviewer>
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/res/leipzig.osm $<TARGET_FILE_DIR:mapviewer>
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/res/jp.osm $<TARGET_FILE_DIR:mapviewer>
)