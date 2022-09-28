
cmake_minimum_required(VERSION 3.21)

project(mf-async)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

aux_source_directory(. PROJECT_SRCS)
file(GLOB PROJECT_HEADERS "*.h")
add_executable (${PROJECT_NAME} ${PROJECT_SRCS} ${PROJECT_HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE mf.lib Mfplat.lib Shlwapi.lib)