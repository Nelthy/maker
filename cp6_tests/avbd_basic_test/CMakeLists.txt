file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(avbd_basic_test ${SRC_FILES})

# adding include files
target_include_directories(avbd_basic_test
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(avbd_basic_test PUBLIC test_common)
target_link_libraries(avbd_basic_test PUBLIC avbd_basic)
target_link_libraries(avbd_basic_test PUBLIC impulse)
target_link_libraries(avbd_basic_test PUBLIC collision_basic_draw)

