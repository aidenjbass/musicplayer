cmake_minimum_required(VERSION 3.10)

project(MusicPlayer)

# Find SDL2, SDL2_mixer, and TagLib via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_MIXER REQUIRED sdl2_mixer)
pkg_check_modules(TAGLIB REQUIRED taglib)

# Ensure we add the pkg-config flags to the CMake variables
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} ${TAGLIB_INCLUDE_DIRS})

# Manually set the library directories for SDL2 and SDL2_mixer
link_directories(${SDL2_LIBRARY_DIRS} ${SDL2_MIXER_LIBRARY_DIRS} ${TAGLIB_LIBRARY_DIRS})

# Check for dependencies
if (NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 not found!")
endif()

if (NOT SDL2_MIXER_FOUND)
    message(FATAL_ERROR "SDL2_mixer not found!")
endif()

if (NOT TAGLIB_FOUND)
    message(FATAL_ERROR "TagLib not found!")
endif()

# Source files
set(SOURCES player.cpp)

# Define executable
add_executable(MusicPlayer ${SOURCES})

# Link libraries
target_link_libraries(MusicPlayer
    ${SDL2_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${TAGLIB_LIBRARIES}
)