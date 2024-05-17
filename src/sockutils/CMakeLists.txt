cmake_minimum_required (VERSION 3.10)

project(sockutils)

aux_source_directory(. DIR_SOURCES)
add_library(${PROJECT_NAME} ${DIR_SOURCES})
