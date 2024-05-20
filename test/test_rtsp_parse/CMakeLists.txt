project(test_rtsp_parse)

include_directories(${project_include_dirs})

aux_source_directory(. DIR_SOURCES)
add_executable(${PROJECT_NAME} ${DIR_SOURCES})

target_link_libraries(${PROJECT_NAME} glog)
target_link_libraries(${PROJECT_NAME} gtest)
target_link_libraries(${PROJECT_NAME} pthread)

target_link_libraries(${PROJECT_NAME} rtsp)
target_link_libraries(${PROJECT_NAME} utils)

gtest_discover_tests(${PROJECT_NAME})
