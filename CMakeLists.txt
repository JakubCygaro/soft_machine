cmake_minimum_required(VERSION 3.80)

project(the_soft_machine VERSION 0.1.2)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(RAYLIB_VERSION 5.5)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include_directories(
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

target_compile_features(cppfeatures INTERFACE cxx_std_23)
target_compile_options(cppfeatures
    INTERFACE -Wall -Wextra
)

find_package(pugixml)

add_executable(soft_machine
    ${CMAKE_SOURCE_DIR}/src/main.cc
    ${CMAKE_SOURCE_DIR}/src/machine/Component.cpp
    ${CMAKE_SOURCE_DIR}/src/machine/Connection.cpp
    ${CMAKE_SOURCE_DIR}/src/machine/MachineContext.cpp
    ${CMAKE_SOURCE_DIR}/src/machine/MachineGraph.cpp
    ${CMAKE_SOURCE_DIR}/src/game/scene.cc
    ${CMAKE_SOURCE_DIR}/src/game/xml.cc
)
target_link_libraries(soft_machine
    PRIVATE cppfeatures
    PRIVATE raylib
    PRIVATE pugixml
)

target_clangformat_setup(soft_machine)

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
