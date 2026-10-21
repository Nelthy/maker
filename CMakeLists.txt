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

add_custom_target(run_avbd_basic_test
    COMMAND avbd_basic_test
    DEPENDS avbd_basic_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    USES_TERMINAL
)

#[[
generate release build
(cmake -G Ninja -S . -B build/gnu_release -DCMAKE_BUILD_TYPE=Release)

generate release build
(cmake -G Ninja -S . -B build/clang_release -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang)


generate debug build
(cmake  -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug)

generate release build
(cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release)


copy compile commands
(cp build/debug/compile_commands.json .)

avbd_basic_test
  DEBUG
  (cmake --build build/debug --config Debug --parallel --target run_avbd_basic_test)
  RELEASE 
  (cmake --build build/release --config Release --parallel --target run_avbd_basic_test)
  
  GNU_RELEASE 
  (cmake --build build/gnu_release --config Release --parallel --target run_avbd_basic_test)
 
  CLANG_RELEASE 
  (cmake --build build/clang_release --config Release --parallel --target run_avbd_basic_test)


#]]
