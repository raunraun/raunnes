cmake_minimum_required(VERSION 3.10)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
project(raunnes)


add_library(glfw3 STATIC IMPORTED)
set_property(TARGET glfw3 PROPERTY IMPORTED_LOCATION ${install_dir}/glfw3.lib)
 
file(GLOB SOURCES "src/*.cpp")
file(GLOB INCLUDES "src/*.h")

add_executable(raunnes ${SOURCES} ${INCLUDES})

