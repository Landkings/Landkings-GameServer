cmake_minimum_required(VERSION 2.8)
project(LandKingsServer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )

file(GLOB headers "include/*.h")
file(GLOB sources "src/*.cpp")
file(GLOB lua "lib/lua/include/*")
file(GLOB lua_wrapper "lib/lua-wrapper/include/*")
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/lua/lib/)
add_executable(${PROJECT_NAME} "main.cpp" ${headers} ${sources} ${lua} ${lua_wrapper})
target_link_libraries(${PROJECT_NAME} "liblua.a")
