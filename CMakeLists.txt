cmake_minimum_required(VERSION 3.10)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
project(raunnes)


add_library(glfw3 STATIC IMPORTED)
set_property(TARGET glfw3 PROPERTY IMPORTED_LOCATION ${install_dir}/glfw3.lib)
 
file(GLOB SOURCES "src/*.cpp")
file(GLOB INCLUDES "src/*.h")

include_directories("deps/glfw-3.3.3.bin.WIN64/include")
link_directories("deps/glfw-3.3.3.bin.WIN64/lib-vc2019")

add_executable(raunnes ${SOURCES} ${INCLUDES})
target_link_libraries(raunnes glfw3)

