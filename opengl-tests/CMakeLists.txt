cmake_minimum_required(VERSION 3.21)

project (opengl-tests)

set(CMAKE_CXX_STANDARD "17")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)

# executable
aux_source_directory(. SRCS)
add_executable(${PROJECT_NAME} ${SRCS})

# glfw
pkg_search_module(GLFW REQUIRED glfw3)
target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${GLFW_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARIES})

# glad
add_subdirectory(deps/glad)
target_link_libraries(${PROJECT_NAME} glad)
