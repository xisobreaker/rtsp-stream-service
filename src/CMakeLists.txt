set(project_include_dirs
    "${CMAKE_HOME_DIRECTORY}/include/rtsputils"
    "${CMAKE_HOME_DIRECTORY}/include/sockutils")

add_subdirectory(sockutils)
add_subdirectory(rtsputils)