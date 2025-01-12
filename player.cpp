#include <SDL.h>
#include <SDL_mixer.h>
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
char KEY_PAUSE = '\''; // Default key for pausing playback
char KEY_NEXT = '.' ; // Default key for playing the next track
char KEY_PREV = ','; // Default key for playing the previous track
char KEY_VOL_UP = ']'; // Default key for increasing volume
char KEY_VOL_DOWN = '['; //Default key for increasing volume
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

// SDL Variables
SDL_GameController *gamepad = NULL;
SDL_Renderer *renderer = NULL;
SDL_Window *window = NULL;
SDL_Texture *osdTexture = NULL;
SDL_Rect osdRect;
int screenWidth = 640, screenHeight = 480;

// User Defined
const char* INI_PATH = "../config.ini";

// Helper function for parsing the ini
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
                KEY_PAUSE = value[0];
            } else if (strcmp(key, "KEY_NEXT") == 0) {
                KEY_NEXT = value[0];
            } else if (strcmp(key, "KEY_PREV") == 0) {
                KEY_PREV = value[0];
            } else if (strcmp(key, "KEY_VOL_UP") == 0) {
                KEY_VOL_UP = value[0];
            } else if (strcmp(key, "KEY_VOL_DOWN") == 0) {
                KEY_VOL_DOWN = value[0];
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

// Function to get music files from a directory
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
        if (strstr(entry->d_name, ".mp3") || 
            strstr(entry->d_name, ".mp4") || 
            strstr(entry->d_name, ".wav") || 
            strstr(entry->d_name, ".wma") || 
            strstr(entry->d_name, ".flac") || 
            strstr(entry->d_name, ".ogg")) {

            // Allocate space for the full path of the file
            char *fullPath = (char *)malloc(strlen(path) + strlen(entry->d_name) + 2);
            snprintf(fullPath, strlen(path) + strlen(entry->d_name) + 2, "%s/%s", path, entry->d_name);

            // Add the file to the list
            files[*trackCount] = fullPath;
            (*trackCount)++;
        }
    }
    closedir(dir);
    return files;
}

// Function to fade volume
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

// Function to load and play music from a given track
void loadAndPlayMusic(int trackIndex) {
    if (music) {
        Mix_FreeMusic(music);
    }
    music = Mix_LoadMUS(trackList[trackIndex]);
    if (music) {
        int startVolume = 0;    
        int endVolume = (INITIAL_VOL * 128) / 100;                 
        Mix_VolumeMusic(startVolume);               
        Mix_PlayMusic(music, -1);
        fadeVolume(startVolume, endVolume, FADE_STEPS, FADE_STEPS_DURATION); // Fade-in effect

    } else {
        printf("LAP Error loading music: %s\n", Mix_GetError());
    }
}

// Function to extract song and artist name from filename
void extractSongAndArtist(const char* filename, char* songName, char* artistName) {
    // Find the last '/' to get the file name without the path
    const char* fileName = strrchr(filename, '/');
    if (!fileName) {
        fileName = filename;  // No path, just the file name
    } else {
        fileName++;  // Skip past the '/'
    }

    // Find the position of the first '-' (assuming the file name format is "songname - artistname.extension")
    const char* dashPos = strchr(fileName, '-');
    if (dashPos) {
        // Copy song name
        size_t songLen = dashPos - fileName;
        strncpy(songName, fileName, songLen);
        songName[songLen] = '\0';

        // Copy artist name (after the dash, until the extension)
        const char* extPos = strchr(dashPos + 1, '.');
        if (extPos) {
            size_t artistLen = extPos - (dashPos + 1);
            strncpy(artistName, dashPos + 1, artistLen);
            artistName[artistLen] = '\0';
        }
    } else {
        // If no '-' is found, treat the whole name as the song name
        strcpy(songName, fileName);
        artistName[0] = '\0'; // No artist name
    }
}

// Function to handle keyboard input
void handleKeyboardInput(SDL_Event *event) {
    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDL_GetKeyFromName(&KEY_VOL_UP)) {
            volume += 10;
            if (volume > 128) volume = 128; // Cap volume at maximum (128)
            Mix_VolumeMusic(volume);
        }

        if (event->key.keysym.sym == SDL_GetKeyFromName(&KEY_VOL_DOWN)) {
            volume -= 10;
            if (volume < 0) volume = 0; // Ensure volume doesn't go below 0
            Mix_VolumeMusic(volume);
        }

        if (event->key.keysym.sym == SDL_GetKeyFromName(&KEY_PAUSE)) {
            // Pause/Play music
            if (Mix_PlayingMusic()) {
                Mix_PauseMusic();
            } else { Mix_ResumeMusic(); }
        }

        if (event->key.keysym.sym == SDL_GetKeyFromName(&KEY_NEXT)) {
            // Next track
            currentTrackIndex = (currentTrackIndex + 1) % trackCount;
            loadAndPlayMusic(currentTrackIndex);
        }

        if (event->key.keysym.sym == SDL_GetKeyFromName(&KEY_PREV)) {
            // Previous track
            currentTrackIndex = (currentTrackIndex - 1 + trackCount) % trackCount;
            loadAndPlayMusic(currentTrackIndex);
        }
    }
}

// Handle gamepad input
void handleGamepadInput() {
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_A)) {
        // Play/Pause
        if (Mix_PlayingMusic()) {
            Mix_PauseMusic();
        } else {
            Mix_ResumeMusic();
        }
    }

    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) {
        // Next track
        currentTrackIndex = (currentTrackIndex + 1) % trackCount;
        loadAndPlayMusic(currentTrackIndex);
    }

    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) {
        // Previous track
        currentTrackIndex = (currentTrackIndex - 1 + trackCount) % trackCount;
        loadAndPlayMusic(currentTrackIndex);
    }

    // Volume control
    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_UP)) {
        volume += 10;
        if (volume > 128) volume = 128;
        Mix_VolumeMusic(volume);
    }

    if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
        volume -= 10;
        if (volume < 0) volume = 0;
        Mix_VolumeMusic(volume);
    }
}

// Function to check if an application is running
int isApplicationOpen(const char* appName) {
    char command[256];
    snprintf(command, sizeof(command), "pgrep -x '%s' > /dev/null", appName);
    return system(command) == 0;  // Returns 1 if the process is running, 0 otherwise
}


void init() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    // Open the first available game controller
    gamepad = SDL_GameControllerOpen(0);
    if (!gamepad) {
        printf("Controller not found!\n");
    }

    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) == 0) {
        printf("Mix_Init failed: %s\n", Mix_GetError());
        exit(1);
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        exit(1);
    }

    // Set initial volume
    if (INITIAL_VOL < 0) INITIAL_VOL = 0;
    if (INITIAL_VOL > 100) INITIAL_VOL = 100;
    Mix_VolumeMusic((INITIAL_VOL * 128) / 100);

    // Load music files
    trackList = getMusicFiles(FOLDER_TO_PLAY, &trackCount);
    if (trackCount == 0) {
        printf("No music files found!\n");
        exit(1);
    }

    // Set shuffle
    if (SHUFFLE) {
        srand(time(NULL));  // Seed the random number generator
        for (int i = trackCount - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            char *temp = trackList[i];
            trackList[i] = trackList[j];
            trackList[j] = temp;
        }
    }

    // Play the first track
    if (trackCount > 0) {
        loadAndPlayMusic(currentTrackIndex);
    }
}

void cleanup() {
    // Free resources
    SDL_DestroyTexture(osdTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main() {
    if (loadSettingsFromFile(INI_PATH) != 0) {
        printf("Failed to load settings from config.ini\n");
        return -1;  // Exit if settings can't be loaded
    }

    init();

    // Main loop
    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
            handleKeyboardInput(&e);
        }
        
        if (PLAY_MUSIC_INGAME) {
            Mix_VolumeMusic(INGAME_VOL);  // Set volume to INGAME_VOL
        } else {
            // If PLAY_MUSIC_INGAME is 0 then we pause music if application is running, demo is WhatsApp
            if (isApplicationOpen("WhatsApp")) {
                if (Mix_PlayingMusic()) {
                    Mix_PauseMusic();
                }
            } else {
                if (Mix_PausedMusic()) {
                    Mix_ResumeMusic();
                }
            }
        }
        handleGamepadInput();
    }

    cleanup();
    return 0;
}