project(test_rtsp)

aux_source_directory(. DIR_SOURCES)
add_executable(${PROJECT_NAME} ${DIR_SOURCES})

target_link_libraries(${PROJECT_NAME} glog)
target_link_libraries(${PROJECT_NAME} gtest)
target_link_libraries(${PROJECT_NAME} pthread)

target_link_libraries(${PROJECT_NAME} rtsp)
target_link_libraries(${PROJECT_NAME} utils)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})