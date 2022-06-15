cmake_minimum_required(VERSION 3.21)

if (WIN32)
  set(CMAKE_C_COMPILER clang-cl)
  set(CMAKE_CXX_COMPILER clang-cl)
elseif (UNIX)
  set(CMAKE_C_COMPILER clang)
  set(CMAKE_CXX_COMPILER clang++)
endif()

project (ffmpeg_tests)

set(CMAKE_CXX_STANDARD "17")
set(CMAKE_EXPORT_COMPILE_COMMANDS "1")

option(ENABLE_TRANSCODER "Enable transcoder" ON)
option(ENABLE_PLAYER "Enable player" ON)


if (WIN32)

    set(FFMPEG_INCLUDE_DIR "e:/ffmpeg-build/build/include")
    include_directories(${FFMPEG_INCLUDE_DIR})
    link_directories(${FFMPEG_INCLUDE_DIR}/../bin ${FFMPEG_INCLUDE_DIR}/../lib)
    # only need to link ffmpeg libraries since all its dependencies have been linked inside
    link_libraries(avfilter avformat avdevice avcodec swresample swscale avutil)
else()

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
    )
    include_directories(${LIBFFMPEG_INCLUDE_DIRS})
    pkg_check_modules(OPENSSL REQUIRED IMPORTED_TARGET openssl)
    link_libraries(PkgConfig::LIBFFMPEG PkgConfig::OPENSSL)

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        link_directories(/usr/local/cuda/lib64/)
        # link_libraries("-lx264")    # apt install libx264-dev or building from source
        link_libraries("-pthread")
    endif()
endif()

# transcoding
if (ENABLE_TRANSCODER)
add_subdirectory(transcoding)
endif()

# player
if (ENABLE_PLAYER)
add_subdirectory(player)
endif()
