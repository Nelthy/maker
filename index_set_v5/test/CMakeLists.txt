cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(index_set_test)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(index_set_test ${SRC_FILES})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. index_set_b)
target_link_libraries(index_set_test PRIVATE index_set)
target_include_directories(index_set_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Debug
#  (cmake -DCMAKE_BUILD_TYPE=Debug  -DCMAKE_C_COMPILER=Clang -DCMAKE_CXX_COMPILER=Clang++ -G Ninja -B build/debug -S .)
#  (cmake --build build/debug --parallel)
#  (build/debug/index_set_test.exe)
#
#  copy compile commands
#  (cp build/debug/compile_commands.json .)


