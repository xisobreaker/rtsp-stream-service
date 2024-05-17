set(project_include_dirs
    "${CMAKE_HOME_DIRECTORY}/include/rtsp"
    "${CMAKE_HOME_DIRECTORY}/include/socket")

add_subdirectory(rtsp)
add_subdirectory(socket)
