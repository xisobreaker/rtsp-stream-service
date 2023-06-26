cmake_minimum_required (VERSION 3.10)

project(rtsp_utils)

add_subdirectory(common)
add_subdirectory(protocol)
add_subdirectory(utils)

aux_source_directory(. DIR_SOURCES)
add_library(${PROJECT_NAME} ${DIR_SOURCES})

target_link_libraries(${PROJECT_NAME} common)
target_link_libraries(${PROJECT_NAME} protocol)
target_link_libraries(${PROJECT_NAME} utils)
