
cmake_minimum_required(VERSION 3.21)

project(gst-tutorials)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

find_package(PkgConfig REQUIRED) 
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
include_directories(${GSTREAMER_INCLUDE_DIRS})
link_directories(${GSTREAMER_LIBRARY_DIRS})
link_libraries(${GSTREAMER_LIBRARIES})

add_executable (basic-tutorial-1 src/basic-tutorial-1.c)
add_executable (basic-tutorial-2 src/basic-tutorial-2.c)
add_executable (basic-tutorial-3 src/basic-tutorial-3.c)
add_executable (simple_initialization src/simple_initialization.c)
add_executable (goption_initialization src/goption_initialization.c)
add_executable (element src/element.c)
add_executable (simple_audio_player src/simple_audio_player.c)
