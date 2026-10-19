cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(index_set INTERFACE)

target_include_directories(index_set
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Debug 
#  (cmake -DCMAKE_BUILD_TYPE=Debug  -DCMAKE_C_COMPILER=Clang -DCMAKE_CXX_COMPILER=Clang++ -G Ninja -B build/debug -S test/)
#  (cmake --build build/debug --parallel ; build/debug/index_set_test)
#  copy compile commands
#  (cp build/debug/compile_commands.json .)
