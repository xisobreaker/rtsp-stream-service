project(test_strutils)

aux_source_directory(. DIR_SOURCES)
add_executable(${PROJECT_NAME} ${DIR_SOURCES})
target_link_libraries(${PROJECT_NAME} common)
target_link_libraries(${PROJECT_NAME} protocol)

add_test(NAME ${PROJECT_NAME} COMMAND ./${PROJECT_NAME})