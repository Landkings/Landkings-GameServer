cmake_minimum_required(VERSION 2.8)

file(GLOB headers "include/*.h")
file(GLOB sources "src/*.cpp")
project(LandKingsServer)
add_executable(${PROJECT_NAME} "main.cpp" ${headers} ${sources})
