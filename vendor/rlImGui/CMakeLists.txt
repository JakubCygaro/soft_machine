cmake_minimum_required(VERSION 3.80)

add_library(rlImGui STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/rlImGui.cpp
)
target_include_directories(rlImGui
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(rlImGui
    PUBLIC raylib
    PUBLIC imgui
)
