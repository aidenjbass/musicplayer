cmake_minimum_required(VERSION 3.10)

# macOS Settings
set(CMAKE_OSX_ARCHITECTURES "arm64")  # Uncomment to force a specific architecture
# set(CMAKE_OSX_ARCHITECTURES "x86_64")  # Uncomment to force x86_64 architecture

project(MusicPlayer)

# macOS platform-specific configuration
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    if (CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        set(SDL2_DIR /opt/homebrew/opt/sdl2)
        set(SDL2_MIXER_DIR /opt/homebrew/opt/sdl2_mixer)
    elseif (CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
        set(SDL2_DIR /usr/local/opt/sdl2)
        set(SDL2_MIXER_DIR /usr/local/opt/sdl2_mixer)
    else()
        message(FATAL_ERROR "Unsupported architecture: ${CMAKE_OSX_ARCHITECTURES}")
    endif()
    # Set CMake prefix path for macOS
    set(CMAKE_PREFIX_PATH ${SDL2_DIR} ${CMAKE_PREFIX_PATH})
endif()

# Find SDL2 and its modules (common for both macOS and Linux)
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

# Includes
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_mixer_INCLUDE_DIRS})

if (NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 not found!")
endif()

if (NOT SDL2_mixer_FOUND)
    message(FATAL_ERROR "SDL2_mixer not found!")
endif()

# Source files
set(SOURCES
    player.cpp
)

# Define executable
add_executable(MusicPlayer ${SOURCES})

# Link libraries
target_link_libraries(MusicPlayer
    SDL2::SDL2
    SDL2_mixer::SDL2_mixer
)