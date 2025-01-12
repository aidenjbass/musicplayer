cmake_minimum_required(VERSION 3.10)

project(MusicPlayer)

set(SDL2_DIR /opt/homebrew/opt/sdl2)
set(SDL2_MIXER_DIR /opt/homebrew/opt/sdl2_mixer)

# headers
include_directories(
    ${SDL2_DIR}/include 
    ${SDL2_MIXER_DIR}/include
    )

# Add source files
set(SOURCES
    player.cpp
)

# Define executable
add_executable(MusicPlayer ${SOURCES})

# Link libraries
target_link_libraries(MusicPlayer
    ${SDL2_DIR}/lib/libSDL2.dylib
    ${SDL2_MIXER_DIR}/lib/libSDL2_mixer.dylib
)
