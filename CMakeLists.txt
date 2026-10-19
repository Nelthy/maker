cmake_minimum_required(VERSION 3.29)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(cp6 LANGUAGES CXX C)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG)
endif()


# Adding subdirectories
add_subdirectory(cp6_tests)
add_subdirectory(cp6_modules)
add_subdirectory(cp6_core)

add_subdirectory(index_set_v5 index_set_b)
add_subdirectory(sfml_state_man_v2 sfml_state_man_b)


#[[
generate debug build
(cmake -G Ninja -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug)

generate release build
(cmake -G Ninja -S . -B build/release -DCMAKE_BUILD_TYPE=Release)

copy compile commands
(cp build/debug/compile_commands.json .)

collision_basic_test
  DEBUG
  (cmake --build build/debug --parallel --target collision_basic_test && ./build/debug/cp6_tests/collision_basic_test/collision_basic_test)

avbd_basic_test
  DEBUG
  (cmake --build build/debug --parallel --target avbd_basic_test && ./build/debug/cp6_tests/avbd_basic_test/avbd_basic_test)

  RELEASE 
  (cmake --build build/release --parallel  --target avbd_basic_test && ./build/release/cp6_tests/avbd_basic_test/avbd_basic_test)
#]]
