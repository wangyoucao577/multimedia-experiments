cmake_minimum_required(VERSION 3.21)

# if (UNIX)
# message("Unix-like OS")
# set(CMAKE_CXX_COMPILER "clang++")
# set(CMAKE_C_COMPILER "clang")
# endif(UNIX)

project (rtmp-flv)

set(CMAKE_CXX_STANDARD "14")
set(CMAKE_EXPORT_COMPILE_COMMANDS "1")

find_package(PkgConfig REQUIRED)

# import rtmp
pkg_check_modules(LIBRTMPDUMP REQUIRED IMPORTED_TARGET librtmp)
include_directories(${LIBRTMPDUMP_INCLUDE_DIRS})
link_libraries(PkgConfig::LIBRTMPDUMP)

aux_source_directory(. RTMP_FLV_SRCS)
file(GLOB RTMP_FLV_HEADERS "*.h")

add_executable (${PROJECT_NAME} ${RTMP_FLV_SRCS} ${RTMP_FLV_HEADERS})
