cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(collision VERSION 1.0)

# adding source files
add_library(collision INTERFACE)

# adding include files
target_include_directories(collision
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(collision INTERFACE cp6_core)

