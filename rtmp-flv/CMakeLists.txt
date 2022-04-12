cmake_minimum_required(VERSION 3.21)

if (UNIX)
message("Unix-like OS")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")
endif(UNIX)

project (rtmp-flv)

set(CMAKE_CXX_STANDARD "14")
set(CMAKE_EXPORT_COMPILE_COMMANDS "1")

find_package(PkgConfig REQUIRED)

# import rtmp
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/Cellar/rtmpdump/2.4+20151223_1/lib/pkgconfig:/usr/local/Cellar/openssl@1.1/1.1.1n/lib/pkgconfig")
pkg_check_modules(LIBRTMPDUMP REQUIRED IMPORTED_TARGET librtmp)
include_directories(${LIBRTMPDUMP_INCLUDE_DIRS})
link_libraries(PkgConfig::LIBRTMPDUMP)

aux_source_directory(. RTMP_FLV_SRCS)
file(GLOB RTMP_FLV_HEADERS "*.h")

add_executable (${PROJECT_NAME} ${RTMP_FLV_SRCS} ${RTMP_FLV_HEADERS})
