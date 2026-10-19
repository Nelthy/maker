cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# adding source files
add_library(test_common ${SRC_FILES})

# adding include files 
target_include_directories(test_common
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(test_common PUBLIC collision_basic)
target_link_libraries(test_common PUBLIC sfml_state_man)

