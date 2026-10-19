cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# adding source files
add_library(collision_basic_draw ${SRC_FILES})

# adding include files
target_include_directories(collision_basic_draw
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(collision_basic_draw PUBLIC collision_basic)
target_link_libraries(collision_basic_draw PUBLIC SFML::Graphics)


