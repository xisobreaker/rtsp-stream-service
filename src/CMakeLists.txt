set(project_include_dirs
    "${CMAKE_HOME_DIRECTORY}/include/rtsp"
    "${CMAKE_HOME_DIRECTORY}/include/socket"
    "${CMAKE_HOME_DIRECTORY}/include/utils")

add_subdirectory(rtsp)
add_subdirectory(socket)
add_subdirectory(utils)
