cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(sfml_state_man VERSION 2.0)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)
# find_package(SFML 3.0.0 COMPONENTS Graphics
#   HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../SFML-3.0.2"
# )

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(sfml_state_man STATIC ${SRC_FILES})

target_include_directories(sfml_state_man PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}/include)

# get_property(_sfml_targets GLOBAL PROPERTY IMPORTED_TARGETS)

target_link_libraries(sfml_state_man PUBLIC SFML::Graphics)



