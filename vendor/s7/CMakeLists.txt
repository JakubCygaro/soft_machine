cmake_minimum_required(VERSION 3.80)

add_library(s7 OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/s7.c
)
target_include_directories(s7
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

