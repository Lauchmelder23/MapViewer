# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("MapViewer")

# Include sub-projects.
add_subdirectory ("vendor/osmparser")
add_subdirectory ("vendor/triangle")
add_subdirectory ("vendor/glad")
add_subdirectory ("vendor/glfw")

add_subdirectory ("src")