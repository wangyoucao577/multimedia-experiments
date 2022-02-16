cmake_minimum_required(VERSION 3.21)

if (UNIX)
message("Unix-like OS")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")
endif(UNIX)

project (ffmpeg_tests)

set(CMAKE_CXX_STANDARD "14")
set(CMAKE_EXPORT_COMPILE_COMMANDS "1")

find_package(PkgConfig REQUIRED)

# import ffmpeg, openssl
pkg_check_modules(LIBFFMPEG REQUIRED IMPORTED_TARGET 
    libavutil
    libavformat
    libavcodec
    libavfilter
    libavdevice
    libswresample
    libswscale
)
include_directories(${LIBFFMPEG_INCLUDE_DIRS})
pkg_check_modules(OPENSSL REQUIRED IMPORTED_TARGET openssl)
link_libraries(PkgConfig::LIBFFMPEG PkgConfig::OPENSSL)

if (UNIX)
link_libraries("-pthread")
endif()

aux_source_directory(transcoding TRANSCODING_SRCS)
add_executable ("transcoding" ${TRANSCODING_SRCS})
