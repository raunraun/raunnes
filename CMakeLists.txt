cmake_minimum_required(VERSION 3.10)
project(raunnes)
 
file(GLOB SOURCES "src/*.cpp")
file(GLOB INCLUDES "src/*.h")

 
add_executable(raunnes ${SOURCES} ${INCLUDES})
