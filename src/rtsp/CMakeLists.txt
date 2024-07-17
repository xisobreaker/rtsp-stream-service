cmake_minimum_required (VERSION 3.10)

project(rtsp)

aux_source_directory(. DIR_SOURCES)
add_library(${PROJECT_NAME} ${DIR_SOURCES})

target_link_libraries(${PROJECT_NAME} glog)
target_link_libraries(${PROJECT_NAME} utils)
