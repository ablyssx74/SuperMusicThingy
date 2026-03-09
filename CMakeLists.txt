cmake_minimum_required(VERSION 3.15)
project(SuperMusicThingy)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CMakeDependentOption)

option(ENABLE_PROJECTM "Enable projectM support" ON)

cmake_dependent_option(ENABLE_SDL2 "Enable SDL2 support" ON "NOT ENABLE_PROJECTM" ON)
cmake_dependent_option(ENABLE_GL   "Enable OpenGL support" ON "NOT ENABLE_PROJECTM" ON)

if(ENABLE_PROJECTM)
    set(ENABLE_SDL2 ON CACHE BOOL "Forced by ProjectM" FORCE)
    set(ENABLE_GL   ON CACHE BOOL "Forced by ProjectM" FORCE)
endif()

add_executable(SuperMusicThingy SuperMusicThingy.cpp)

find_package(PkgConfig REQUIRED)

if(HAIKU)
    list(APPEND CMAKE_LIBRARY_PATH /boot/home/config/non-packaged/lib)
    list(APPEND CMAKE_INCLUDE_PATH /boot/home/config/non-packaged/include)
    target_include_directories(SuperMusicThingy PRIVATE /boot/home/config/non-packaged/include)
endif()

pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)
pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)
find_library(OPENAL_LIB openal REQUIRED)

target_link_libraries(SuperMusicThingy PRIVATE
    PkgConfig::MPV
    PkgConfig::CURL
    ${OPENAL_LIB}
)

if(ENABLE_SDL2)
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
    target_link_libraries(SuperMusicThingy PRIVATE PkgConfig::SDL2)
    target_compile_definitions(SuperMusicThingy PRIVATE USE_SDL2)
endif()

if(ENABLE_PROJECTM)
    # 1. Use pkg-config to get the correct include paths and library hints
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PM4 projectM-4) # Note: Removed IMPORTED_TARGET for more control

    if(PM4_FOUND)
        # 2. Use the hints from pkg-config to find the actual library file
        find_library(PROJECTM_LIBRARY
            NAMES projectM-4 projectM libprojectM-4
            HINTS ${PM4_LIBRARY_DIRS}
        )

        if(PROJECTM_LIBRARY)
            target_link_libraries(SuperMusicThingy PRIVATE ${PROJECTM_LIBRARY})
            target_include_directories(SuperMusicThingy PRIVATE ${PM4_INCLUDE_DIRS})
            target_compile_definitions(SuperMusicThingy PRIVATE USE_PROJECTM)
            message(STATUS "Found projectM: ${PROJECTM_LIBRARY}")
        else()
            message(FATAL_ERROR "Found projectM-4.pc but couldn't find the .so file!")
        endif()
    else()
        message(FATAL_ERROR "projectM-4.pc not found. Install libprojectm-dev.")
    endif()
endif()


if(ENABLE_GL)
    find_package(OpenGL REQUIRED)
    target_link_libraries(SuperMusicThingy PRIVATE OpenGL::GL)
    target_compile_definitions(SuperMusicThingy PRIVATE USE_GL)
endif()
