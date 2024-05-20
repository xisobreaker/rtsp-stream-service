cmake_minimum_required (VERSION 3.10)

project(rtsp_stream_service VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)
find_package(GLog REQUIRED)

add_subdirectory(src)
aux_source_directory(. DIR_SOURCES)
add_executable(${PROJECT_NAME} ${DIR_SOURCES})
target_link_libraries(${PROJECT_NAME} glog::glog)

include(CTest)
include(GoogleTest)
if(BUILD_TESTING)
    message(STATUS "Build testing...")
    enable_testing()
    add_subdirectory(test)
endif()
