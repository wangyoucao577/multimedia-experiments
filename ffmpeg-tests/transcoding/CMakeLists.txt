
cmake_minimum_required(VERSION 3.21)

project(transcoding)

aux_source_directory(. TRANSCODING_SRCS)
file(GLOB TRANSCODING_HEADERS "*.h")
add_executable (${PROJECT_NAME} ${TRANSCODING_SRCS} ${TRANSCODING_HEADERS})
