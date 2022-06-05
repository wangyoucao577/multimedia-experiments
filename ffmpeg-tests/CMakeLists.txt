cmake_minimum_required(VERSION 3.21)

if (UNIX)
message("Unix-like OS")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")
endif(UNIX)

project (ffmpeg_tests)

set(CMAKE_CXX_STANDARD "14")
set(CMAKE_EXPORT_COMPILE_COMMANDS "1")

# use my ffmpeg build
set(ENV{PKG_CONFIG_PATH} "~/workspace/ffmpeg-build/build/lib/pkgconfig")
if (APPLE)
# brew installed libs normal has prefix '/usr/local', which can be retrieved by `brew --prefix` 
set(ENV{PKG_CONFIG_PATH} "/usr/local/opt/openssl/lib/pkgconfig:ENV{PKG_CONFIG_PATH}")
endif()

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
    x264
    sdl2
)
include_directories(${LIBFFMPEG_INCLUDE_DIRS})
pkg_check_modules(OPENSSL REQUIRED IMPORTED_TARGET openssl)
link_libraries(PkgConfig::LIBFFMPEG PkgConfig::OPENSSL)

if (UNIX)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
link_directories(/usr/local/cuda/lib64/)
# link_libraries("-lx264")    # apt install libx264-dev or building from source
link_libraries("-pthread")
endif()
endif()

# transcoding
add_subdirectory(transcoding)

