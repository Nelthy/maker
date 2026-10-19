# This is mainly here to create compile commands
cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# adding subdirectorys
add_subdirectory(collision)
add_subdirectory(collision_basic)
add_subdirectory(collision_basic_draw)

add_subdirectory(avbd)
add_subdirectory(avbd_basic)

add_subdirectory(impulse)

