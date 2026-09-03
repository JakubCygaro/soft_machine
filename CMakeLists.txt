cmake_minimum_required(VERSION 3.80)

project(the_soft_machine VERSION 0.1.2)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(RAYLIB_VERSION 5.5)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include_directories(
    BEFORE ${CMAKE_SOURCE_DIR}/vendor/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_options(-D DEBUG)
endif()

add_library(cppfeatures INTERFACE)

include(ClangFormat)
include(FetchContent)
# #Commands are left empty so that we only checkout the source and no not perform any kind of build
# FetchContent_Declare(gitmeu3
#     GIT_REPOSITORY "https://github.com/JakubCygaro/meurglys3.git"
#     GIT_TAG 8ac9e5d1bba82c67528376ce5410ad33b992a1e2
# )
# FetchContent_MakeAvailable(gitmeu3)
#
# add_library(meu3 INTERFACE IMPORTED)
# set_target_properties(
#     meu3
#     PROPERTIES
#     INTERFACE_INCLUDE_DIRECTORIES "${meu3IncludeDirs}"
# )
# set_property(TARGET meu3
#     APPEND
#     PROPERTY
#     INTERFACE_LINK_LIBRARIES "${meu3StaticLibs}"
#     APPEND
#     PROPERTY
#     INTERFACE_LINK_LIBRARIES "${meu3LinkLibs}"
# )
# include(FetchContent)

find_package(raylib ${RAYLIB_VERSION})
if (NOT raylib_FOUND)
    FetchContent_Declare(
        raylib
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
        URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_MakeAvailable(raylib)
endif()

target_compile_features(cppfeatures INTERFACE cxx_std_26)
target_compile_options(cppfeatures
    INTERFACE -Wall -Wextra -freflection
)

find_package(pugixml)

add_library(sf_commons STATIC
    ${CMAKE_SOURCE_DIR}/src/machine/Component.cpp
    ${CMAKE_SOURCE_DIR}/src/machine/Connection.cpp
    ${CMAKE_SOURCE_DIR}/src/machine/MachineContext.cpp
    ${CMAKE_SOURCE_DIR}/src/machine/MachineGraph.cpp
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
)

add_executable(soft_machine
    ${CMAKE_SOURCE_DIR}/src/main.cc
)
target_link_libraries(soft_machine
    PRIVATE raylib
    PRIVATE pugixml
    PRIVATE sf_commons
)

find_library(
    rlimgui
    rlImGui
    PATHS ${CMAKE_SOURCE_DIR}/vendor/lib
    NO_DEFAULT_PATH
    REQUIRED
)
add_executable(facelift
    ${CMAKE_SOURCE_DIR}/src/facelift/main.cc
    ${CMAKE_SOURCE_DIR}/src/facelift/gather.cc
)
target_compile_definitions(facelift
    PRIVATE -DFACELIFT_EDITOR
)

target_link_libraries(facelift
    PRIVATE raylib
    PRIVATE pugixml
    PRIVATE ${rlimgui}
    PRIVATE sf_commons
)

target_clangformat_setup(soft_machine)
target_clangformat_setup(facelift)

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(bootleg
        PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/subsystem windows>
        PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wl,-subsystem,windows>
    )
endif()

# configure_file(version.h.in version.h @ONLY)

# find_program(hasMeu3 meurglys3 OPTIONAL)
#
# if(hasMeu3 AND ${CMAKE_BUILD_TYPE} STREQUAL "Release" AND NOT NO_PACKAGE_GAMEDATA)
#     message(STATUS "Attempting to package gamedata for a release build")
#     execute_process(
#         COMMAND ${hasMeu3} pack ${CMAKE_CURRENT_SOURCE_DIR}/gamedata ${CMAKE_CURRENT_BINARY_DIR}/gamedata
#         ERROR_VARIABLE packErr
#         ECHO_ERROR_VARIABLE
#     )
#     if(packErr)
#         message(WARNING "There was an error while trying to package gamedata")
#         message(WARNING ${packErr})
#     else()
#         message(STATUS "Packaging successfull")
#     endif()
# endif()
