cmake_minimum_required(VERSION 3.10)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
project(raunnes)

find_package(SFML COMPONENTS window graphics system)

file(GLOB SOURCES "src/*.cpp")
file(GLOB INCLUDES "src/*.h")

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")

add_executable(raunnes ${SOURCES} ${INCLUDES})
target_include_directories(${PROJECT_NAME} PRIVATE "/home/raun/Code/raunnes/deps") 
target_link_libraries(${PROJECT_NAME}  sfml-graphics)

