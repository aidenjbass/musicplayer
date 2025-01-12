#include </opt/homebrew/include/SDL2/SDL.h>
#include </opt/homebrew/include/SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

// Settings 
#define FOLDER_TO_PLAY "/Users/aiden/Library/CloudStorage/OneDrive-Personal/Music/Bleachers/Live At Bowery Ballroom (Amazon Music Songline)"
#define FILE_TYPES "mp3|mp4|wav|wma|flac|ogg"
#define M3U_FILE_NAME "playlist.m3u"
#define INITIAL_VOL 100
#define INGAME_VOL 100
#define SHUFFLE 1
#define RESUME_NEXT_TRACK 0
#define KEY_PAUSE "'"  // Pause key
#define KEY_NEXT "."   // Next track key
#define KEY_PREV ","   // Previous track key
#define KEY_VOL_UP "]" // Volume up key
#define KEY_VOL_DOWN "[" // Volume down key
#define OSD 0 // OSD flag (0 = no, 1 = show track info)
#define SHOW_TRACK_INFO 1

// Music Player Variables
Mix_Music *music = NULL;
int volume = INITIAL_VOL;
int currentTrackIndex = 0;
int trackCount = 0;
char **trackList = NULL;
int shuffle = SHUFFLE;
SDL_GameController *gamepad = NULL;
int playMusicInGame = 0;

// Function to get music files from a folder and return them as a list
char **getMusicFiles(const char *path, int *trackCount) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("Could not open directory");
        return NULL;
    }

    struct dirent *entry;
    char **files = (char **)malloc(100 * sizeof(char *));
    *trackCount = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            // Check if the file matches the allowed file types
            if (strstr(entry->d_name, ".mp3") || strstr(entry->d_name, ".wav") || strstr(entry->d_name, ".ogg")) {
                files[*trackCount] = strdup(entry->d_name);
                (*trackCount)++;
            }
        }
    }

    closedir(dir);
    return files;
}

// Function to load and play music from a given track list
void loadAndPlayMusic(int trackIndex) {
    if (music) {
        Mix_FreeMusic(music);
    }

    char trackPath[1024];
    snprintf(trackPath, sizeof(trackPath), "%s/%s", FOLDER_TO_PLAY, trackList[trackIndex]);
    music = Mix_LoadMUS(trackPath);
    if (music) {
        Mix_PlayMusic(music, -1); // Loop indefinitely
    }
}

// Function to handle keyboard input
void handleKeyboardInput(SDL_Event *event) {
    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_q) {
            // Quit the program
            SDL_Quit();
            exit(0);
        }

        if (event->key.keysym.sym == SDLK_r) {
            // Shuffle the playlist
            if (shuffle) {
                // Shuffle the trackList
                srand(time(NULL));
                for (int i = trackCount - 1; i > 0; i--) {
                    int j = rand() % (i + 1);
                    char *temp = trackList[i];
                    trackList[i] = trackList[j];
                    trackList[j] = temp;
                }
            }
        }

        if (event->key.keysym.sym == SDLK_UP) {
            // Increase volume
            volume += 10;
            if (volume > 128) volume = 128;
            Mix_VolumeMusic(volume);
        }
        if (event->key.keysym.sym == SDLK_DOWN) {
            // Decrease volume
            volume -= 10;
            if (volume < 0) volume = 0;
            Mix_VolumeMusic(volume);
        }

        if (event->key.keysym.sym == SDLK_p) {
            // Pause/Play music
            if (Mix_PlayingMusic()) {
                Mix_PauseMusic();
            } else {
                Mix_ResumeMusic();
            }
        }

        if (event->key.keysym.sym == SDLK_n) {
            // Next track
            currentTrackIndex = (currentTrackIndex + 1) % trackCount;
            loadAndPlayMusic(currentTrackIndex);
        }

        if (event->key.keysym.sym == SDLK_b) {
            // Previous track
            currentTrackIndex = (currentTrackIndex - 1 + trackCount) % trackCount;
            loadAndPlayMusic(currentTrackIndex);
        }
    }
}

// Initialize the gamepad
void initGamepad() {
    if (SDL_NumJoysticks() > 0) {
        gamepad = SDL_GameControllerOpen(0);
    }
}

// Handle gamepad input
void handleGamepadInput() {
    if (!gamepad) return;

    // Gamepad button mappings
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_A)) {
        // Play/Pause
        if (Mix_PlayingMusic()) {
            Mix_PauseMusic();
        } else {
            Mix_ResumeMusic();
        }
    }

    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_B)) {
        // Next track
        currentTrackIndex = (currentTrackIndex + 1) % trackCount;
        loadAndPlayMusic(currentTrackIndex);
    }

    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_X)) {
        // Previous track
        currentTrackIndex = (currentTrackIndex - 1 + trackCount) % trackCount;
        loadAndPlayMusic(currentTrackIndex);
    }

    // Volume control
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) {
        volume += 10;
        if (volume > 128) volume = 128;
        Mix_VolumeMusic(volume);
    }

    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) {
        volume -= 10;
        if (volume < 0) volume = 0;
        Mix_VolumeMusic(volume);
    }
}

// Main function
int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_FLAC) == 0) {
        printf("Mix_Init failed: %s\n", Mix_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        return 1;
    }

    // Get music files
    trackList = getMusicFiles(FOLDER_TO_PLAY, &trackCount);
    if (!trackList || trackCount == 0) {
        printf("No music files found\n");
        return 1;
    }

    // Load and play the first track
    loadAndPlayMusic(currentTrackIndex);

    // Initialize gamepad
    initGamepad();

    // Main event loop
    SDL_Event e;
    while (1) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                SDL_Quit();
                exit(0);
            }
            handleKeyboardInput(&e);
        }

        // Handle gamepad input
        handleGamepadInput();
    }

    SDL_Quit();
    return 0;
}
