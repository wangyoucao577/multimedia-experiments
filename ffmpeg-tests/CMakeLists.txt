cmake_minimum_required(VERSION 3.21)

if (UNIX)
message("Unix-like OS")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")
endif(UNIX)

project (ffmpeg_tests)

set(CMAKE_CXX_STANDARD "14")
set(CMAKE_EXPORT_COMPILE_COMMANDS "1")

find_package(PkgConfig)

pkg_check_modules(LIBAVUTIL REQUIRED libavutil)
pkg_check_modules(LIBAVFORMAT REQUIRED libavformat)
pkg_check_modules(LIBAVCODEC REQUIRED libavcodec)
pkg_check_modules(LIBAVFILTER REQUIRED libavfilter)
pkg_check_modules(LIBAVDEVICE REQUIRED libavdevice)
pkg_check_modules(LIBSWRESAMPLE REQUIRED libswresample)
pkg_check_modules(LIBSWSCALE REQUIRED libswscale)
include_directories(
    ${LIBAVUTIL_INCLUDE_DIRS} 
    ${LIBAVFORMAT_INCLUDE_DIRS} 
    ${LIBAVCODEC_INCLUDE_DIRS} 
    ${LIBAVFILTER_INCLUDE_DIRS}
    ${LIBAVDEVICE_INCLUDE_DIRS}
    ${LIBSWRESAMPLE_INCLUDE_DIRS}
    ${LIBSWSCALE_INCLUDE_DIRS}
)
link_directories(
    ${LIBAVUTIL_LIBRARY_DIRS} 
    ${LIBAVFORMAT_LIBRARY_DIRS} 
    ${LIBAVCODEC_LIBRARY_DIRS} 
    ${LIBAVFILTER_LIBRARY_DIRS}
    ${LIBAVDEVICE_LIBRARY_DIRS}
    ${LIBSWRESAMPLE_LIBRARY_DIRS}
    ${LIBSWSCALE_LIBRARY_DIRS}
)
link_libraries(
    ${LIBAVFORMAT_LIBRARIES}
    ${LIBAVCODEC_LIBRARIES} 
    ${LIBAVFILTER_LIBRARIES}
    ${LIBAVDEVICE_LIBRARIES}
    ${LIBSWRESAMPLE_LIBRARIES}
    ${LIBSWSCALE_LIBRARIES}
    ${LIBAVUTIL_LIBRARIES} 
)

aux_source_directory(transcoding TRANSCODING_SRCS)
add_executable ("transcoding" ${TRANSCODING_SRCS})
