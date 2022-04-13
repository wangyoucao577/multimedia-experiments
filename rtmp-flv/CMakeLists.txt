cmake_minimum_required(VERSION 3.21)

project (rtmp-flv)

set(CMAKE_CXX_STANDARD "14")
set(CMAKE_EXPORT_COMPILE_COMMANDS "1")

find_package(PkgConfig REQUIRED)

if (APPLE)
# for mac, `brew install rtmpdump` installed librtmp will requires libssl
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/Cellar/rtmpdump/2.4+20151223_1/lib/pkgconfig:/usr/local/Cellar/openssl@1.1/1.1.1n/lib/pkgconfig")
endif()

# librtmp installation
#   linux: `apt install librtmp-dev`
#   mac:   `brew install rtmpdump`
pkg_check_modules(LIBRTMPDUMP REQUIRED IMPORTED_TARGET librtmp)
include_directories(${LIBRTMPDUMP_INCLUDE_DIRS})
link_libraries(PkgConfig::LIBRTMPDUMP)

aux_source_directory(. RTMP_FLV_SRCS)
file(GLOB RTMP_FLV_HEADERS "*.h")

add_executable (${PROJECT_NAME} ${RTMP_FLV_SRCS} ${RTMP_FLV_HEADERS})
