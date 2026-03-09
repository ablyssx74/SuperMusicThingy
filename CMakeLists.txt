cmake_minimum_required(VERSION 3.15)
project(SuperMusicThingy)

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

# Only add Haiku paths if we are actually on Haiku
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
    # On Linux, try pkg-config first as it's more reliable for projectM-4
    pkg_check_modules(PROJECTM libprojectM-4 IMPORTED_TARGET)

    if(TARGET PkgConfig::PROJECTM)
        target_link_libraries(SuperMusicThingy PRIVATE PkgConfig::PROJECTM)
        target_compile_definitions(SuperMusicThingy PRIVATE USE_PROJECTM)
    else()
        # Fallback to find_library (useful for Haiku or custom Linux builds)
        find_library(PROJECTM_LIBRARY NAMES projectM-4)

        if(PROJECTM_LIBRARY)
            target_link_libraries(SuperMusicThingy PRIVATE ${PROJECTM_LIBRARY})
            target_compile_definitions(SuperMusicThingy PRIVATE USE_PROJECTM)
        else()
            message(FATAL_ERROR "projectM-4 library not found. Install libprojectm-dev or similar.")
        endif()
    endif()
endif()

if(ENABLE_GL)
    find_package(OpenGL REQUIRED)
    target_link_libraries(SuperMusicThingy PRIVATE OpenGL::GL)
    target_compile_definitions(SuperMusicThingy PRIVATE USE_GL)
endif()
