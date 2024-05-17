project(test_utils)

aux_source_directory(. DIR_SOURCES)
add_executable(${PROJECT_NAME} ${DIR_SOURCES})

target_link_libraries(${PROJECT_NAME} glog)
target_link_libraries(${PROJECT_NAME} gtest)
target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME} common)
target_link_libraries(${PROJECT_NAME} protocol)

gtest_discover_tests(${PROJECT_NAME})
