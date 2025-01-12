cmake_minimum_required(VERSION 3.10)

project(MusicPlayer)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SDL2_DIR /opt/homebrew/opt/sdl2)
    set(SDL2_MIXER_DIR /opt/homebrew/opt/sdl2_mixer)
    set(SDL2_TTF_DIR /opt/homebrew/opt/sdl2_ttf)
endif()

# headers
include_directories(
    ${SDL2_DIR}/include 
    ${SDL2_MIXER_DIR}/include
    ${SDL2_TTF_DIR}/include
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
    ${SDL2_TTF_DIR}/lib/libSDL2_ttf.dylib
)
