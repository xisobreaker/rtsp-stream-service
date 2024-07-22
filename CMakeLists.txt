cmake_minimum_required (VERSION 3.10)

project(rtsp_stream_service VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)

find_package(GTest REQUIRED)
find_package(GLog REQUIRED)

set(project_include_dirs
    "${CMAKE_HOME_DIRECTORY}/include/manager"
    "${CMAKE_HOME_DIRECTORY}/include/rtsp"
    "${CMAKE_HOME_DIRECTORY}/include/socket"
    "${CMAKE_HOME_DIRECTORY}/include/utils")
include_directories(${project_include_dirs})
add_subdirectory(src)

aux_source_directory(. DIR_SOURCES)
add_executable(${PROJECT_NAME} ${DIR_SOURCES})
target_link_libraries(${PROJECT_NAME} glog::glog)
target_link_libraries(${PROJECT_NAME} manager)
target_link_libraries(${PROJECT_NAME} socket)
target_link_libraries(${PROJECT_NAME} utils)
target_link_libraries(${PROJECT_NAME} rtsp)

include(CTest)
if(BUILD_TESTING)
    message(STATUS "Build testing...")
    enable_testing()
    add_subdirectory(test)
endif()
