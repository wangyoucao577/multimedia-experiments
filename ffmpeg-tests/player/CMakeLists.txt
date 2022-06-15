
cmake_minimum_required(VERSION 3.21)

project(player)

aux_source_directory(. PLAYER_SRCS)
file(GLOB PLAYER_HEADERS "*.h")
add_executable (${PROJECT_NAME} ${PLAYER_SRCS} ${PLAYER_HEADERS})

if (WIN32)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third-party/SDL2/SDL2-devel-2.0.22-VC/SDL2-2.0.22/include)
    # link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third-party/SDL2-devel-2.0.22-VC/SDL2-2.0.22/lib/x64)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/SDL2/SDL2-devel-2.0.22-VC/SDL2-2.0.22/lib/x64/SDL2.lib)
else()
    pkg_check_modules(LIBSDL2 REQUIRED IMPORTED_TARGET sdl2)
    target_link_libraries(${PROJECT_NAME} PkgConfig::LIBSDL2)
endif()
