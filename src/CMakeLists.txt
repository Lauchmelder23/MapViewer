cmake_minimum_required(VERSION 3.10)

add_executable(mapviewer
    main.cpp
    Multipolygon.cpp
	Window.cpp
)

target_link_libraries(mapviewer PRIVATE 
	osmparser
	triangle
	glfw
    glad
)

add_custom_command(TARGET mapviewer POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/res/map.osm $<TARGET_FILE_DIR:mapviewer>
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/res/bigmap.osm $<TARGET_FILE_DIR:mapviewer>
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/res/leipzig.osm $<TARGET_FILE_DIR:mapviewer>
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/res/jp.osm $<TARGET_FILE_DIR:mapviewer>
)