cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(avbd_basic VERSION 1.0)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# adding source files
add_library(avbd_basic ${SRC_FILES})

# adding include files
target_include_directories(avbd_basic
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(avbd_basic PUBLIC avbd)
target_link_libraries(avbd_basic PUBLIC collision_basic)


