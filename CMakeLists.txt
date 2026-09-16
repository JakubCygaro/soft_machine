cmake_minimum_required(VERSION 3.80)

project(the_soft_machine VERSION 0.1.2)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(RAYLIB_VERSION 5.5)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include_directories(
    # ${CMAKE_SOURCE_DIR}/vendor/rlImGui
    # ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/include
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_options(-D DEBUG)
endif()

add_library(cppfeatures INTERFACE)

include(ClangFormat)
include(FetchContent)

find_package(raylib ${RAYLIB_VERSION})
if (NOT raylib_FOUND)
    FetchContent_Declare(
        raylib
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
        URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_MakeAvailable(raylib)
endif()

add_subdirectory(vendor)

target_compile_features(cppfeatures INTERFACE cxx_std_26)
target_compile_options(cppfeatures
    INTERFACE -Wall -Wextra -freflection
)

find_package(pugixml)

# find_library(
#     rlimgui
#     librlImGui.a
#     PATHS ${CMAKE_SOURCE_DIR}/vendor/lib
#     NO_DEFAULT_PATH
#     REQUIRED
# )

add_library(sf_commons STATIC
    ${CMAKE_SOURCE_DIR}/src/machine/Component.cpp
    ${CMAKE_SOURCE_DIR}/src/machine/Connection.cpp
    ${CMAKE_SOURCE_DIR}/src/machine/MachineContext.cpp
    ${CMAKE_SOURCE_DIR}/src/game/scene.cc
    ${CMAKE_SOURCE_DIR}/src/game/xml.cc
    ${CMAKE_SOURCE_DIR}/src/components/Passthrough.cc
    ${CMAKE_SOURCE_DIR}/src/components/Memory.cc
    ${CMAKE_SOURCE_DIR}/src/components/Cpu.cc
    ${CMAKE_SOURCE_DIR}/src/components/Button.cc
    ${CMAKE_SOURCE_DIR}/src/components/Display.cc
    ${CMAKE_SOURCE_DIR}/src/components/Repeater.cc
    ${CMAKE_SOURCE_DIR}/src/game/resources/resources.cc
    ${CMAKE_SOURCE_DIR}/include/misc/cpp/imgui_stdlib.cpp
)
target_link_libraries(sf_commons
    PUBLIC cppfeatures
    # PRIVATE ${rlimgui}
    PUBLIC rlImGui
    PUBLIC imgui
)

add_executable(soft_machine
    ${CMAKE_SOURCE_DIR}/src/main.cc
)
target_link_libraries(soft_machine
    PRIVATE raylib
    PRIVATE pugixml
    PRIVATE sf_commons
)

add_executable(facelift
    ${CMAKE_SOURCE_DIR}/src/facelift/main.cc
    ${CMAKE_SOURCE_DIR}/src/facelift/gather.cc
    ${CMAKE_SOURCE_DIR}/src/facelift/state.cc
)
target_compile_definitions(facelift
    PRIVATE -DFACELIFT_EDITOR
)
target_include_directories(facelift
    PUBLIC ${CMAKE_SOURCE_DIR}/vendor/rlImGui
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
)

target_link_libraries(facelift
    PRIVATE raylib
    PRIVATE pugixml
    PRIVATE sf_commons
)

target_clangformat_setup(soft_machine)
target_clangformat_setup(facelift)
