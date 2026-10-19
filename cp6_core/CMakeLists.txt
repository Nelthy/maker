cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# adding source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(cp6_core STATIC ${SRC_FILES})

# adding include files
target_include_directories(cp6_core
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(cp6_core PUBLIC index_set)

