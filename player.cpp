#include </opt/homebrew/include/SDL2/SDL.h>
#include </opt/homebrew/include/SDL2/SDL_mixer.h>
#include </opt/homebrew/include/SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

// Settings variables
char FOLDER_TO_PLAY[1024];
char PLAYLIST_FILE_PATH[256];
int FADE_STEPS;
int FADE_STEPS_DURATION;
int INITIAL_VOL;
int PLAY_MUSIC_INGAME;
int INGAME_VOL;
int SHUFFLE;
int RESUME_NEXT_TRACK;
int KEYBOARD_CONTROL;
int GAMEPAD_CONTROL_FULL;
int GAMEPAD_CONTROL_SIMPLE;
int GAMEPAD_CONTROL_PAUSE;
int GAMEPAD_CONTROL_L1R1;
char KEY_PAUSE[2];
char KEY_NEXT[2];
char KEY_PREV[2];
char KEY_VOL_UP[2];
char KEY_VOL_DOWN[2];
int OSD;
int OSD_SCREEN;
int OSD_PERCENTFROMBOTTOM;
int SHOW_TRACK_INFO;

// Music Player Variables
Mix_Music *music = NULL;
int volume = 0;
int currentTrackIndex = 0;
int trackCount = 0;
char **trackList = NULL;

SDL_GameController *gamepad = NULL;
TTF_Font *font = NULL;
SDL_Renderer *renderer = NULL;
SDL_Window *window = NULL;
SDL_Texture *osdTexture = NULL;
SDL_Rect osdRect;
int screenWidth = 640, screenHeight = 480; // Adjust to your screen size

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
                // Create the full path to the file
                char *fullPath = (char *)malloc(strlen(path) + strlen(entry->d_name) + 2);
                snprintf(fullPath, strlen(path) + strlen(entry->d_name) + 2, "%s/%s", path, entry->d_name);
                files[*trackCount] = fullPath;
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
    music = Mix_LoadMUS(trackList[trackIndex]);
    if (music) {
        Mix_PlayMusic(music, -1); // Loop indefinitely
    } else {
        printf("LAP Error loading music: %s\n", Mix_GetError());
    }
}


void fadeVolume(int startVolume, int endVolume, int steps, int duration) {
    int volumeChange = (endVolume - startVolume) / steps;
    int delay = duration / steps;  // Time delay between each step

    for (int i = 0; i < steps; i++) {
        volume += volumeChange;
        if (volume > 128) volume = 128;  // Cap at max volume
        if (volume < 0) volume = 0;     // Prevent going below 0
        Mix_VolumeMusic(volume);
        SDL_Delay(delay);  // Delay to simulate fade over time
    }
}

void loadSystemFont() {
    const char* fontPath = "/System/Library/Fonts/Helvetica.ttc"; 
    font = TTF_OpenFont(fontPath, 24);  // Load font with size 24
    if (font == NULL) {
        printf("Failed to load system font! SDL_ttf Error: %s\n", TTF_GetError());
    }
}

void renderOSD(const char* songName, const char* artistName) {
    // Combine song and artist into one string
    char text[512];
    snprintf(text, sizeof(text), "%s - %s", songName, artistName);

    // Free any existing texture
    if (osdTexture) {
        SDL_DestroyTexture(osdTexture);
    }

    // Define the color for text (white)
    SDL_Color white = {255, 255, 255, 255};  // White color with full opacity

    // Create surface from text
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, white);
    if (!textSurface) {
        printf("Unable to create text surface: %s\n", TTF_GetError());
        return;
    }

    // Create texture from surface
    osdTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!osdTexture) {
        printf("Unable to create texture from surface: %s\n", SDL_GetError());
    }

    // Set OSD rectangle (centered at the bottom of the screen)
    int osdHeight = textSurface->h;
    int osdWidth = textSurface->w;

    int offsetY = (screenHeight * OSD_PERCENTFROMBOTTOM) / 100;
    osdRect.x = (screenWidth - osdWidth) / 2;  // Center horizontally
    osdRect.y = screenHeight - osdHeight - offsetY;  // Pad from bottom
    osdRect.w = osdWidth;
    osdRect.h = osdHeight;

    // Free the surface after creating the texture
    SDL_FreeSurface(textSurface);
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
            if (SHUFFLE) {
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

void stripQuotes(char *str) {
    int len = strlen(str);
    // Check if the string starts and ends with a quote and strip them
    if (len > 1 && str[0] == '"' && str[len - 1] == '"') {
        // Shift the string to remove the quotes
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0'; // Null terminate the string after removing the quotes
    }
    // Remove extra spaces around the value
    char *start = str;
    while (*start == ' ') start++;  // Remove leading spaces
    char *end = str + strlen(str) - 1;
    while (end > start && *end == ' ') end--;  // Remove trailing spaces
    *(end + 1) = '\0';  // Null terminate the string
}

// Function to parse the INI file
int loadSettingsFromFile(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error opening INI file: %s\n", filename);
        return -1;
    }

    char line[512];
    char section[64] = {0};
    while (fgets(line, sizeof(line), file)) {
        // Remove newline characters
        line[strcspn(line, "\n")] = 0;
        
        // Ignore empty lines and comments
        if (line[0] == '\0' || line[0] == ';') continue;

        // Parse key-value pairs
        else if (strchr(line, '=')) {
            char key[128], value[256];
            sscanf(line, "%127[^=]=%255[^\n]", key, value);

            // Remove quotes from the value if present
            stripQuotes(value);

            // Assign values to settings based on key
            if (strcmp(key, "FOLDER_TO_PLAY") == 0) {
                strncpy(FOLDER_TO_PLAY, value, sizeof(FOLDER_TO_PLAY) - 1);
            } else if (strcmp(key, "PLAYLIST_FILE_PATH") == 0) {
                strncpy(PLAYLIST_FILE_PATH, value, sizeof(PLAYLIST_FILE_PATH) - 1);
            } else if (strcmp(key, "FADE_STEPS") == 0) {
                FADE_STEPS = atoi(value); // Convert to integer
            } else if (strcmp(key, "FADE_STEPS_DURATION") == 0) {
            FADE_STEPS_DURATION = atoi(value); // Convert to integer
            } else if (strcmp(key, "INITIAL_VOL") == 0) {
                INITIAL_VOL = atoi(value); // Convert to integer
            } else if (strcmp(key, "PLAY_MUSIC_INGAME") == 0) {
            PLAY_MUSIC_INGAME = atoi(value); // Convert to integer
            } else if (strcmp(key, "INGAME_VOL") == 0) {
                INGAME_VOL = atoi(value); // Convert to integer
            } else if (strcmp(key, "SHUFFLE") == 0) {
                SHUFFLE = atoi(value); // Convert to integer
            } else if (strcmp(key, "RESUME_NEXT_TRACK") == 0) {
                RESUME_NEXT_TRACK = atoi(value); // Convert to integer
            } else if (strcmp(key, "KEYBOARD_CONTROL") == 0) {
                KEYBOARD_CONTROL = atoi(value); // Convert to integer
            } else if (strcmp(key, "GAMEPAD_CONTROL_FULL") == 0) {
                GAMEPAD_CONTROL_FULL = atoi(value); // Convert to integer
            } else if (strcmp(key, "GAMEPAD_CONTROL_SIMPLE") == 0) {
                GAMEPAD_CONTROL_SIMPLE = atoi(value); // Convert to integer
            } else if (strcmp(key, "GAMEPAD_CONTROL_PAUSE") == 0) {
                GAMEPAD_CONTROL_PAUSE = atoi(value); // Convert to integer
            } else if (strcmp(key, "GAMEPAD_CONTROL_L1R1") == 0) {
                GAMEPAD_CONTROL_L1R1 = atoi(value); // Convert to integer
            } else if (strcmp(key, "KEY_PAUSE") == 0) {
                strncpy(KEY_PAUSE, value, sizeof(KEY_PAUSE) - 1);
            } else if (strcmp(key, "KEY_NEXT") == 0) {
                strncpy(KEY_NEXT, value, sizeof(KEY_NEXT) - 1);
            } else if (strcmp(key, "KEY_PREV") == 0) {
                strncpy(KEY_PREV, value, sizeof(KEY_PREV) - 1);
            } else if (strcmp(key, "KEY_VOL_UP") == 0) {
                strncpy(KEY_VOL_UP, value, sizeof(KEY_VOL_UP) - 1);
            } else if (strcmp(key, "KEY_VOL_DOWN") == 0) {
                strncpy(KEY_VOL_DOWN, value, sizeof(KEY_VOL_DOWN) - 1);
            } else if (strcmp(key, "OSD") == 0) {
                OSD = atoi(value); // Convert to integer
            } else if (strcmp(key, "SHOW_TRACK_INFO") == 0) {
                SHOW_TRACK_INFO = atoi(value); // Convert to integer
            }
        }
    }

    fclose(file);
    return 0;
}

void init() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }

    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) == 0) {
        printf("Mix_Init failed: %s\n", Mix_GetError());
        exit(1);
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        exit(1);
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        exit(1);
    }

    font = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 24);  // Use a valid font path
    if (!font) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        exit(1);
    }

    window = SDL_CreateWindow("Music Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_BORDERLESS);
    if (!window) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        exit(1);
    }

    // Load music files
    trackList = getMusicFiles(FOLDER_TO_PLAY, &trackCount);  // Replace with actual folder path
    if (trackCount == 0) {
        printf("No music files found!\n");
        exit(1);
    }

    // Shuffle the playlist if SHUFFLE is enabled
    if (SHUFFLE) {
        srand(time(NULL));  // Seed the random number generator
        for (int i = trackCount - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            // Swap the elements
            char *temp = trackList[i];
            trackList[i] = trackList[j];
            trackList[j] = temp;
        }
    }


    // Play the first track
    if (trackCount > 0) {
        char trackPath[1024];
        snprintf(trackPath, sizeof(trackPath), "%s", trackList[currentTrackIndex]);
        music = Mix_LoadMUS(trackPath);
        if (music) {
            Mix_PlayMusic(music, -1);  // Play looped music
        } else {
            printf("Error loading music: %s\n", Mix_GetError());
        }
    }
}
void cleanup() {
    // Free resources
    SDL_DestroyTexture(osdTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void displayOSD() {
    SDL_RenderClear(renderer);  // Clear the screen

    // Render the OSD (text overlay)
    if (osdTexture) {
        SDL_RenderCopy(renderer, osdTexture, NULL, &osdRect);  // Copy the texture to the renderer
    }

    SDL_RenderPresent(renderer);  // Present the renderer
}

int main() {
    if (loadSettingsFromFile("../config.ini") != 0) {
        printf("Failed to load settings from config.ini\n");
        return -1;  // Exit if settings can't be loaded
    }

    init();

    const char* songName = "Song Title";
    const char* artistName = "Artist Name";

    renderOSD(songName, artistName);  // Prepare the overlay text

    // Main loop
    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        // Display the OSD
        displayOSD();
        SDL_Delay(16);  // Delay to limit the frame rate
    }

    cleanup();
    return 0;
}