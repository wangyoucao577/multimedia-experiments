
cmake_minimum_required(VERSION 3.21)

project(player)

aux_source_directory(. PLAYER_SRCS)
file(GLOB PLAYER_HEADERS "*.h")
add_executable (${PROJECT_NAME} ${PLAYER_SRCS} ${PLAYER_HEADERS})
