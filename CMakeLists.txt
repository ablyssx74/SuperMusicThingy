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
    list(APPEND CMAKE_INCLUDE_PATH /boot/home/config/non-packaged/lib/pkgconfig)      
    target_include_directories(SuperMusicThingy PRIVATE /boot/home/config/non-packaged/include)
endif()

pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)

pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)

find_path(JSON_INCLUDE_DIR NAMES nlohmann/json.hpp
    HINTS /usr/local/include /usr/include /boot/home/config/non-packaged/include
)

if(JSON_INCLUDE_DIR)
    target_include_directories(SuperMusicThingy PRIVATE ${JSON_INCLUDE_DIR})
    message(STATUS "Found nlohmann-json headers at: ${JSON_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "nlohmann/json.hpp not found! Please install nlohmann-json3-dev (Linux) or nlohmann_json (Haiku).")
endif()


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
    find_package(PkgConfig REQUIRED)
    set(MY_PKG_PATHS "/usr/lib/pkgconfig:/usr/local/lib/pkgconfig:/boot/home/config/non-packaged/lib/pkgconfig:/tmp/SuperMusicThingy/hpkgs/SuperMusicThingyNebula/lib/pkgconfig")
    
    if(HAIKU)
        set(DUMMY_PC_PATH "${CMAKE_BINARY_DIR}/pkgconfig")
        file(MAKE_DIRECTORY "${DUMMY_PC_PATH}")
        file(WRITE "${DUMMY_PC_PATH}/opengl.pc" 
"Name: opengl
Description: Dummy OpenGL for Haiku
Version: 1.0
Libs: -L/boot/system/lib -lGL
Cflags: -I/boot/system/develop/headers")
        

        set(MY_PKG_PATHS "${DUMMY_PC_PATH}:${MY_PKG_PATHS}")
    endif()


    set(ENV{PKG_CONFIG_PATH} "${MY_PKG_PATHS}:$ENV{PKG_CONFIG_PATH}")


    pkg_check_modules(PM4 projectM-4)

    if(PM4_FOUND)

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
