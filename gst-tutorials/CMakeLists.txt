
cmake_minimum_required(VERSION 3.21)

if (Darwin)
message("macOSX")
endif()

project(gst-tutorials)

find_package(PkgConfig REQUIRED) 
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
include_directories(${GSTREAMER_INCLUDE_DIRS})
link_directories(${GSTREAMER_LIBRARY_DIRS})
link_libraries(${GSTREAMER_LIBRARIES})

add_executable (basic-tutorial-1 src/basic-tutorial-1.c)
add_executable (basic-tutorial-2 src/basic-tutorial-2.c)

