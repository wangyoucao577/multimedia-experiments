
project (glad)

aux_source_directory(src GLAD_SRCS)
add_library(${PROJECT_NAME} ${GLAD_SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC include)
