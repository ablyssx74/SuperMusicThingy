#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sys/ioctl.h>
#include <unistd.h>
#include <curl/curl.h>
#include <mpv/client.h>
#include "nlohmann/json.hpp"
#include <poll.h>
#include <csignal>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <limits.h>
#include <sstream>
#include <fcntl.h>

#ifdef USE_SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#endif

#ifdef USE_GL
#include <GL/gl.h>
#endif

#ifdef USE_PROJECTM
#include <projectM-4/projectM.h>
#endif

#include <mutex>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <random>

namespace fs = std::filesystem;
// --- Global State ---

uint32_t lastPresetChange = 0;
const uint32_t PRESET_DURATION = 30000;
void update_visuals_logic();
std::string currentPresetName = "None";
#ifdef __HAIKU__
#include <image.h>
#include <OS.h>
#include <AL/al.h>
#include <AL/alc.h>
ALCdevice *alcCaptureDevice = nullptr;
#elif defined(USE_SDL2)
SDL_AudioDeviceID captureDevice = 0;
#endif


void cleanup_capture_device() {
    #ifdef __HAIKU__
    if (alcCaptureDevice) {
        alcCaptureCloseDevice(alcCaptureDevice);
        alcCaptureDevice = nullptr;
    }
    #elif defined(USE_SDL2)
    if (captureDevice > 0) {
        SDL_CloseAudioDevice(captureDevice);
        captureDevice = 0;
    }
    #endif
}



std::time_t saveMessageTimer = 0;
#ifdef USE_PROJECTM
projectm_handle pm = nullptr;
#endif
bool needsRedraw = true;
bool visualsRunning = false;

#ifdef USE_SDL2
SDL_Window* visualWin = nullptr;
SDL_GLContext glContext = nullptr;
#endif

float audioBuffer[2048];

// --- Global UI Colors ---
const std::string BLUE   = "\033[94m";
const std::string RED    = "\033[91m";
const std::string ORANGE = "\033[93m";
const std::string WHITE  = "\033[97m";
const std::string YELLOW = "\033[33m";
const std::string GREEN  = "\033[38;5;46m";
const std::string RESET  = "\033[0m";

std::string get_ui_header(int rows) {
    std::stringstream header;
    header << "\033[40m" << "\033[H\033[2J\033[3J" << BLUE;
    header << "\033[2;10H" << "             SuperMusicThingy\n";
    header << "\033[3;10H" << "[S]huffle | Vol [+/-] | [H]elp | [Q]uit\n";
    header << "\033[4;10H" << "[j/k] Scroll | [Enter] Update/Play | [B]ack\n";
    return header.str();
}


std::string get_ui_footer(int rows) {
    std::stringstream footer;
    struct winsize w; ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    footer << "\033[" << w.ws_row << ";0H" << RED << "SuperMusicThingy~ $: ";
    return footer.str();
}


std::string statusMsg = "";
std::time_t statusExpiry = 0;
const std::string BASE_URL = "https://somafm.com/";
using json = nlohmann::json;
int selectedFav = 0;
int scrollOffset = 0;
bool showMenu = false;
bool showHelp = false;
bool showNotifications = false;
bool showConfig = false;

// Path for the config file
#ifdef __HAIKU__
std::string configPath = getenv("HOME") + std::string("/config/settings/MusicThingy/config.txt");
#else
std::string configPath = getenv("HOME") + std::string("/.config/MusicThingy/config.txt");
#endif


struct Config {
    bool showNotifications = true;
    bool showVisuals = false;
    bool autoShuffle = false;
    bool startMuted = false;
    int defaultVolume = 100;
    std::string quality = "high";
} cfg;

int selectedConfig = 0;


void save_config() {
    json j;
    j["quality"] = cfg.quality;
    j["showNotifications"] = cfg.showNotifications;
    j["autoShuffle"] = cfg.autoShuffle;
    j["startMuted"] = cfg.startMuted;
    j["showVisuals"] = cfg.showVisuals;

    std::ofstream outfile(configPath);
    outfile << j.dump(4);
}

void load_config() {
    std::ifstream infile(configPath);
    if (infile.is_open()) {
        try {
            json j = json::parse(infile);
            cfg.quality = j.value("quality", "highest");
            cfg.showNotifications = j.value("showNotifications", true);
            cfg.autoShuffle = j.value("autoShuffle", false);
            cfg.showVisuals = j.value("showVisuals", true);
            cfg.startMuted = j.value("startMuted", false);
        } catch(...) {}
    }
}



// --- For reading arguments from keyboard shortcuts ---
const char* fifoPath = "/tmp/musicthingy_fifo";
const char* respPath = "/tmp/musicthingy_resp";
int fifoFd = -1;

// Delete fifo on exit
void cleanup_fifo() {
    unlink(fifoPath);
    unlink(respPath);
}

// --- OS Path Helper ---
std::string get_self_path() {
    char buffer[PATH_MAX];
    #ifdef __HAIKU__
    image_info info;
    int32 cookie = 0;
    while (get_next_image_info(0, &cookie, &info) == B_OK) {
        if (info.type == B_APP_IMAGE) return std::string(info.name);
    }
    #else
    // Linux/Unix
    ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
    if (count > 0) return std::string(buffer, count);
    #endif
    return "";
}


struct Channel {
    std::string title;
    std::string id;
    std::string desc;
    std::string listeners;
};

mpv_handle *mpv = nullptr;
std::vector<Channel> channels;
volatile sig_atomic_t resized = 0;

std::string pendingSong = "";
std::time_t notifyTimer = 0;

std::string currentSong = "None";
std::string currentDesc = "None";
std::string currentStation = "Press [s] to shuffle!";
std::string currentListeners = "";

// --- Helper Functions ---
void handle_resize(int sig) { resized = 1; }

bool kbhit() {
    struct pollfd fds; fds.fd = STDIN_FILENO; fds.events = POLLIN;
    return poll(&fds, 1, 0) > 0;
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// --- Logic Functions ---

void fetch_channels() {
    channels.clear();
    CURL* curl = curl_easy_init();
    std::string buffer;
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, (BASE_URL + "channels.json").c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "MusicThingy/1.0");
        if(curl_easy_perform(curl) == CURLE_OK) {
            try {
                auto data = json::parse(buffer);
                for (auto& ch : data["channels"]) {
                    channels.push_back({
                        ch.value("title", ""),
                                       ch.value("id", ""),
                                       ch.value("description", ""),
                                       ch.value("listeners", "0")
                    });
                }
            } catch(...) {}
        }
        curl_easy_cleanup(curl);
    }
}


#ifdef USE_PROJECTM
void load_random_preset(projectm_handle pm) {
    const char* home = getenv("HOME");
    if (!home) return; 
#ifdef __HAIKU__
    std::string configPath = std::string(home) + "/config/settings/MusicThingy/milk_presets/";
#else
    std::string configPath = std::string(home) + "/.config/MusicThingy/milk_presets/";
#endif

    std::vector<std::string> presets;

    try {

        if (!std::filesystem::exists(configPath)) {
            std::filesystem::create_directories(configPath);
            return; 
        }

        // Change from directory_iterator to recursive_directory_iterator
        for (const auto& entry : std::filesystem::recursive_directory_iterator(configPath)) {
            // is_regular_file ensures we don't try to "load" a folder as a preset
            if (entry.is_regular_file() && entry.path().extension() == ".milk") {
                presets.push_back(entry.path().string());
            }
        }


        if (presets.empty()) {
            std::cerr << "No presets found in: " << configPath << std::endl;
            return;
        }

		static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
        std::uniform_int_distribution<int> dist(0, presets.size() - 1);
        std::string selected = presets[dist(rng)];

        projectm_load_preset_file(pm, selected.c_str(), true);
        currentPresetName = std::filesystem::path(selected).filename().string();

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "FS Error: " << e.what() << std::endl;
    }
}
#endif


#ifdef USE_PROJECTM
void init_visuals() {
    // 1.
#ifndef __HAIKU__
    setenv("SDL_PULSEAUDIO_INCLUDE_MONITORS", "1", 1);
    setenv("SDL_AUDIODRIVER", "pulseaudio", 1);       
#endif

    // 2.
#ifdef __HAIKU__
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
#else
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
#endif
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        return;
    }

    // 3.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // 4.
    visualWin = SDL_CreateWindow("Music Thingy Visuals",
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!visualWin) return;

    glContext = SDL_GL_CreateContext(visualWin);
    SDL_GL_MakeCurrent(visualWin, glContext);
    
    // Disable VSync for better responsiveness on Haiku
    SDL_GL_SetSwapInterval(0);

    // 5. THE "EARS" LOGIC
#ifdef __HAIKU__
    alcCaptureDevice = alcCaptureOpenDevice(NULL, 48000, AL_FORMAT_STEREO16, 8192);
    if (!alcCaptureDevice) {
        // HAIL MARY: Open the "null" backend just to get a node in Cortex
        alcCaptureDevice = alcCaptureOpenDevice("null", 48000, AL_FORMAT_STEREO16, 8192);
    }
    
    if (alcCaptureDevice) {
        alcCaptureStart(alcCaptureDevice);
    }

#else

    // Linux PulseAudio logic (unchanged)
    SDL_Delay(100);
    if (captureDevice == 0) {
        SDL_AudioSpec wanted;
        SDL_zero(wanted);
        wanted.freq = 44100;
        wanted.format = AUDIO_F32;
        wanted.channels = 2;
        wanted.samples = 1024;
        int count = SDL_GetNumAudioDevices(1);
        const char* monitorDeviceName = NULL;
        for (int i = 0; i < count; ++i) {
            const char* name = SDL_GetAudioDeviceName(i, 1);
            if (name && (strstr(name, "monitor") || strstr(name, "Monitor"))) {
                monitorDeviceName = name;
                break;
            }
        }
        captureDevice = SDL_OpenAudioDevice(monitorDeviceName, 1, &wanted, NULL, 0);
        if (captureDevice > 0) SDL_PauseAudioDevice(captureDevice, 0);
    }
#endif


    // 6. Initialize projectM
    pm = projectm_create();
    if (pm) {
        projectm_set_window_size(pm, 800, 600);
        load_random_preset(pm);
        lastPresetChange = SDL_GetTicks();
        visualsRunning = true;
    }
}
#endif


void init_mpv() {
    mpv = mpv_create();
    if (!mpv) exit(1);
    #ifdef __HAIKU__
    mpv_set_option_string(mpv, "ao", "openal");
    #else
    mpv_set_option_string(mpv, "ao", "pulse");
    #endif

    mpv_set_option_string(mpv, "input-default-bindings", "yes");
    mpv_set_option_string(mpv, "terminal", "no");
    if (mpv_initialize(mpv) < 0) exit(1);
    mpv_observe_property(mpv, 0, "media-title", MPV_FORMAT_STRING);
    mpv_observe_property(mpv, 0, "paused-for-cache", MPV_FORMAT_FLAG);
}




void fade_volume(mpv_handle *mpv, double target_vol, double duration_ms) {
    double current_vol;
    mpv_get_property(mpv, "volume", MPV_FORMAT_DOUBLE, &current_vol);

    int steps = 20; // Number of small volume jumps
    double step_size = (target_vol - current_vol) / steps;
    int step_duration = (int)(duration_ms * 1000 / steps); // in microseconds

    for (int i = 0; i < steps; ++i) {
        current_vol += step_size;
        mpv_set_property(mpv, "volume", MPV_FORMAT_DOUBLE, &current_vol);
        usleep(step_duration);
    }
    // Ensure we hit the exact target
    mpv_set_property(mpv, "volume", MPV_FORMAT_DOUBLE, &target_vol);
}

std::string get_quality_url(const std::string& id) {
    if (cfg.quality == "highest") return BASE_URL + id + "256.pls"; // 256k MP3
    if (cfg.quality == "low")     return BASE_URL + id + "64.pls";  // 64k AAC-HE
    return BASE_URL + id + "130.pls"; // Default High (128k AAC)
}

void play_random() {
    if (channels.empty()) return;

    // 1.
    double original_vol;
    mpv_get_property(mpv, "volume", MPV_FORMAT_DOUBLE, &original_vol);
    fade_volume(mpv, 0, 300);

    // 2.
    int idx = rand() % channels.size();
    currentStation = channels[idx].title;
    currentDesc = channels[idx].desc;
    currentListeners = channels[idx].listeners;
    currentSong = "Buffering...";

    // USE THE HELPER
    std::string url = get_quality_url(channels[idx].id);

    const char *cmd[] = {"loadfile", url.c_str(), NULL};
    mpv_command(mpv, cmd);

    // 3.
    fade_volume(mpv, original_vol, 500);
}



void set_volume(char direction) {
    double vol;
    mpv_get_property(mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
    if (direction == '+') vol += 5;
    if (direction == '-') vol -= 5;
    if (vol > 100) vol = 100; if (vol < 0) vol = 0;
    mpv_set_property(mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
}

void toggle_mute() {
    int mute;
    mpv_get_property(mpv, "mute", MPV_FORMAT_FLAG, &mute);
    mute = !mute;
    mpv_set_property(mpv, "mute", MPV_FORMAT_FLAG, &mute);
}

int count_favorites() {
    std::string home = getenv("HOME") ? getenv("HOME") : ".";
    #ifdef __HAIKU__
    std::ifstream infile(home + "/config/settings/MusicThingy/favorites.txt");
    #else
    std::ifstream infile(home + "/.config/MusicThingy/favorites.txt");
    #endif
    int lines = 0;
    std::string line;
    while (std::getline(infile, line)) if (!line.empty()) lines++;
    return lines;
}

bool is_favorite() {
    std::string home = getenv("HOME") ? getenv("HOME") : ".";
    #ifdef __HAIKU__
    std::ifstream infile(home + "/config/settings/MusicThingy/favorites.txt");
    #else
    std::ifstream infile(home + "/.config/MusicThingy/favorites.txt");
    #endif


    std::string currentUrl = "";
    for(const auto& ch : channels) {
        if(ch.title == currentStation) {
            currentUrl = BASE_URL + ch.id + ".pls";
            break;
        }
    }

    if (currentUrl.empty()) return false;

    std::string line;
    while (std::getline(infile, line)) {
        if (line == currentUrl) return true;
    }
    return false;
}

std::string get_vol_bar() {
    double vol;
    mpv_get_property(mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
    int filled = (int)(vol / 10);
    std::string bar = "[";
    for (int i = 0; i < 10; ++i) {
        if (i < filled) bar += "|";
        else bar += ".";
    }
    bar += "]";
    return bar;
}


bool draw_help_menu() {
    struct winsize w; ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    std::stringstream buffer;

    buffer << get_ui_header(w.ws_row);

    int r = 10;
    buffer << "\033[" << r++ << ";10H [s] Shuffle      : Play a random station";
    buffer << "\033[" << r++ << ";10H [f] Play Fav     : Play a random favorite";
    buffer << "\033[" << r++ << ";10H [l] List Favs    : Open scrollable favorite menu";
    buffer << "\033[" << r++ << ";10H [a] Add Fav      : Save current station to list";
    buffer << "\033[" << r++ << ";10H [d] Delete Fav   : Remove current station from list";
    buffer << "\033[" << r++ << ";10H [+/-] Volume     : Increase/Decrease volume";
    buffer << "\033[" << r++ << ";10H [m] Mute         : Toggle audio mute";
    buffer << "\033[" << r++ << ";10H [j/k] Scroll     : Scroll up/down selection";
    buffer << "\033[" << r++ << ";10H [enter] Play     : Play or Update selection";
    buffer << "\033[" << r++ << ";10H [v] Shuffle      : Shuffle milk drop presets";
    buffer << "\033[" << r++ << ";10H [k] Fullscreen   : Fullscreen visual effects window";
    buffer << "\033[" << r++ << ";10H [h] Help         : Show this menu";
    buffer << "\033[" << r++ << ";10H [c] Config       : Config Manager";
    buffer << "\033[" << r++ << ";10H [q] Quit         : Exit Music Thingy";

    buffer << get_ui_footer(w.ws_row);
    buffer << RESET;

    std::cout << buffer.str() << std::flush;

    if (kbhit()) {
        char c = getchar();
        if (c == 'b' || c == 27 || c == 'h') {
            return false; // Tell main loop to CLOSE the menu
        }
    }
    return true; // Keep the menu OPEN
}



void update_metadata_from_url(const std::string& url) {
    for (const auto& ch : channels) {
        // Match the channel ID within the URL string
        if (url.find(ch.id) != std::string::npos) {
            currentStation = ch.title;
            currentDesc = ch.desc;
            currentListeners = ch.listeners;
            currentSong = "Loading Favorite...";
            break;

        }
    }
}

bool draw_favorites_menu() {
    struct winsize w; ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    std::stringstream buffer;

    // 1. Load favorites
    std::string home = getenv("HOME") ? getenv("HOME") : ".";
    #ifdef __HAIKU__
    std::ifstream infile(home + "/config/settings/MusicThingy/favorites.txt");
    #else
    std::ifstream infile(home + "/.config/MusicThingy/favorites.txt");
    #endif

    std::vector<std::string> favUrls;
    std::string line;
    while (std::getline(infile, line)) if (!line.empty()) favUrls.push_back(line);

    // 2. Build UI

    buffer << get_ui_header(w.ws_row);

    int maxVisible = 10;
    if (favUrls.empty()) {
        buffer << "\033[7;14H  (No favorites saved yet)";
    } else {
        if (selectedFav < scrollOffset) scrollOffset = selectedFav;
        if (selectedFav >= scrollOffset + maxVisible) scrollOffset = selectedFav - maxVisible + 1;

        for (int i = 0; i < maxVisible && (i + scrollOffset) < (int)favUrls.size(); ++i) {
            int idx = i + scrollOffset;
            buffer << "\033[" << (10 + i) << ";10H";
            if (idx == selectedFav) buffer << BLUE << " > " <<  ORANGE << favUrls[idx] << BLUE;
            else buffer << "   " << favUrls[idx];
        }
    }

    buffer << get_ui_footer(w.ws_row);
    buffer << RESET;
    std::cout << buffer.str() << std::flush;

    // 3. Handle Input
    if (kbhit()) {
        char c = getchar();
        if (c == 'b' || c == 27) return false; // Exit menu
        if (c == 'k' && selectedFav > 0) selectedFav--;
        if (c == 'j' && selectedFav < (int)favUrls.size() - 1) selectedFav++;

        if ((c == '\n' || c == '\r') && !favUrls.empty()) {
            const char *cmd[] = {"loadfile", favUrls[selectedFav].c_str(), NULL};
            mpv_command(mpv, cmd);

            // Manually update metadata here or call your helper
            for (const auto& ch : channels) {
                if (favUrls[selectedFav].find(ch.id) != std::string::npos) {
                    currentStation = ch.title;
                    currentDesc = ch.desc;
                    break;
                }
            }
            return false; // Exit menu after playing
        }
    }

    return true; // Keep menu open if no exit key was pressed
}

// Rainbow Text template
//[\033[31mF\033[33ma\033[32mv\033[36mo\033[34m\033[35mr\033[31mi\033[33mt\033[32me\033[94m]

std::string get_bitrate_text() {
    if (cfg.quality == "highest") return "256k";
    if (cfg.quality == "low")     return "64k";
    return "128k"; // Default for "high"
}


void draw_ui() {
    struct winsize w; ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    std::stringstream buffer;

    int mute;
    mpv_get_property(mpv, "mute", MPV_FORMAT_FLAG, &mute);
    std::string volColor = mute ? "\033[91m" : "\033[92m"; // Red if muted

    // Build the frame in memory
    buffer << "\033[H\033[2J\033[3J"; // Full Clear

    buffer << get_ui_header(w.ws_row);

    if (std::time(nullptr) < statusExpiry) {
        buffer << "\033[" << (w.ws_row - 16) << ";10H" << GREEN << ">> " << statusMsg << "\n" << RESET << BLUE ;
    }

    if (!currentSong.empty() && currentSong != "None")
        buffer << "\033[" << (w.ws_row - 15) << ";10H" << " * " << currentSong << "\n";

    if (!currentDesc.empty() && currentDesc != "None" && currentDesc != "None") {
        buffer << "\033[" << (w.ws_row - 14) << ";10H" << " * " << currentDesc;
    }

    buffer << "\033[" << (w.ws_row - 13) << ";10H" << BLUE << " * "  << currentStation;
    if (is_favorite()) buffer << " " << "[\033[31mF\033[33ma\033[32mv\033[36mo\033[34m\033[35mr\033[31mi\033[33mt\033[32me\033[94m]" << RESET;

    if(!currentListeners.empty()) buffer << "\033[" << (w.ws_row - 12) << ";10H" << BLUE << " * Listeners: " << currentListeners;

    buffer << "\033[" << (w.ws_row - 11) << ";10H"  << " * Total Channels: " << (int)channels.size();

    buffer << "\033[" << (w.ws_row - 10) << ";10H" << " * Favorites: " << count_favorites() << "\n";

    buffer << "\033[" << (w.ws_row - 9) << ";10H" << " * Bitrate: " << get_bitrate_text() << "\n";

    buffer << "\033[" << (w.ws_row - 8) << ";10H" << " * Vol: " << volColor << get_vol_bar() << "\n";

    #ifdef USE_PROJECTM
    if (visualsRunning) {
        buffer << "\033[" << (w.ws_row - 7) << ";10H" << BLUE << " * Milkdrop: " << volColor << currentPresetName << "\033[K\n";
    }
    #endif


    buffer << get_ui_footer(w.ws_row);

    buffer << RESET;

    std::cout << buffer.str() << std::flush;
}

void list_favorites() {
    std::string home = getenv("HOME") ? getenv("HOME") : ".";
    #ifdef __HAIKU__
    std::string path = home + "/config/settings/MusicThingy/favorites.txt";
    #else
    std::string path = home + "/.config/MusicThingy/favorites.txt";
    #endif

    std::ifstream infile(path);
    if (!infile.is_open()) {
        statusMsg = "No favorites file found.";
        statusExpiry = std::time(nullptr) + 3;
        return;
    }

    std::string line;
    std::string listStr = "Favorites: ";
    bool first = true;

    while (std::getline(infile, line)) {
        if (line.empty()) continue;

        // Extract the ID from the URL (e.g., "groovesalad" from ".../groovesalad.pls")
        size_t lastSlash = line.find_last_of('/');
        size_t lastDot = line.find_last_of('.');
        if (lastSlash != std::string::npos && lastDot != std::string::npos) {
            std::string id = line.substr(lastSlash + 1, lastDot - lastSlash - 1);

            // Find the human-readable title from your channels vector
            for (const auto& ch : channels) {
                if (ch.id == id) {
                    if (!first) listStr += ", ";
                    listStr += ch.title;
                    first = false;
                    break;
                }
            }
        }
    }

    if (first) statusMsg = "Your favorites list is empty.";
    else statusMsg = listStr;

    statusExpiry = std::time(nullptr) + 10; // Show for 10 seconds
}


void save_favorite() {
    std::string home = getenv("HOME") ? getenv("HOME") : ".";
    #ifdef __HAIKU__
    std::string dir = home + "/config/settings/MusicThingy";
    std::string path = dir + "/favorites.txt";
    #else
    std::string dir = home + "/.config/MusicThingy";
    std::string path = dir + "/favorites.txt";
    #endif

    mkdir(dir.c_str(), 0755);

    // 1. Determine the URL for the current station
    std::string currentUrl = "";
    for(const auto& ch : channels) {
        if(ch.title == currentStation) {
            currentUrl = BASE_URL + ch.id + ".pls";
            break;
        }
    }

    if (currentUrl.empty()) {
        statusMsg = "Cannot save: No station selected.";
        statusExpiry = std::time(nullptr) + 2;
        return;
    }

    // 2. Check if URL already exists in the file
    std::ifstream infile(path);
    std::string line;
    bool isDuplicate = false;
    while (std::getline(infile, line)) {
        if (line == currentUrl) {
            isDuplicate = true;
            break;
        }
    }
    infile.close();

    // 3. Save only if it's NOT a duplicate
    if (isDuplicate) {
        statusMsg = "Already in favorites!";
    } else {
        std::ofstream outfile(path, std::ios_base::app);
        if (outfile.is_open()) {
            outfile << currentUrl << std::endl;
            statusMsg = "URL saved to favorites!";
            outfile.close();
        } else {
            statusMsg = "Error opening file!";
        }
    }
    statusExpiry = std::time(nullptr) + 2;
}


void play_favorite() {
    std::string home = getenv("HOME") ? getenv("HOME") : ".";
    #ifdef __HAIKU__
    std::string path = home + "/config/settings/MusicThingy/favorites.txt";
    #else
    std::string path = home + "/.config/MusicThingy/favorites.txt";
    #endif

    std::ifstream infile(path);
    std::vector<std::string> favs;
    std::string line;
    while (std::getline(infile, line)) if (!line.empty()) favs.push_back(line);

    if (favs.empty()) {
        statusMsg = "No favorites saved!";
        statusExpiry = std::time(nullptr) + 2;
        return;
    }

    std::string url = favs[rand() % favs.size()];

    // Extract ID from URL to update global state correctly
    // URL format: https://somafm.com
    size_t lastSlash = url.find_last_of('/');
    size_t lastDot = url.find_last_of('.');
    if (lastSlash != std::string::npos && lastDot != std::string::npos) {
        std::string id = url.substr(lastSlash + 1, lastDot - lastSlash - 1);
        for (const auto& ch : channels) {
            if (ch.id == id) {
                currentStation = ch.title;
                currentDesc = ch.desc;
                currentListeners = ch.listeners;
                break;
            }
        }
    }

    currentSong = "Loading Favorite...";
    const char *cmd[] = {"loadfile", url.c_str(), NULL};
    mpv_command(mpv, cmd);
}


void delete_favorite() {
    std::string home = getenv("HOME") ? getenv("HOME") : ".";
    #ifdef __HAIKU__
    std::string path = home + "/config/settings/MusicThingy/favorites.txt";
    #else
    std::string path = home + "/.config/MusicThingy/favorites.txt";
    #endif


    std::string currentUrl = "";
    for(const auto& ch : channels) {
        if(ch.title == currentStation) {
            currentUrl = BASE_URL + ch.id + ".pls";
            break;
        }
    }


    if (currentUrl.empty()) return;

    std::ifstream infile(path);
    std::vector<std::string> remaining;
    std::string line;
    bool removed = false;

    while (std::getline(infile, line)) {
        if (line != currentUrl && !line.empty()) remaining.push_back(line);
        else removed = true;
    }
    infile.close();

    if (removed) {
        std::ofstream outfile(path, std::ios::trunc);
        for (const auto& f : remaining) outfile << f << "\n";
        statusMsg = "Deleted from favorites.";
    } else {
        statusMsg = "Not in favorites.";
    }
    statusExpiry = std::time(nullptr) + 2;
}

void send_notification(const std::string& station, const std::string& song) {

    // Filter out common URL patterns to prevent "URL notifications"
    if (song.empty() || song.find("http://") == 0 || song.find("https://") == 0 || song.find("-aac") != std::string::npos || song.find("-mp3") != std::string::npos) {
        return;
    }

    std::string cmd;
    #ifdef __HAIKU__
    cmd = "notify --title \"Music Thingy\" \"" + station + ": " + song + "\" &";
    #else
    cmd = "notify-send \"Music Thingy\" \"" + station + "\n" + song + "\" &";
    #endif
    system(cmd.c_str());
}


bool draw_config_menu() {
    struct winsize w; ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    std::stringstream buffer;
    buffer << get_ui_header(w.ws_row);

    // Define the list of options to display
    struct MenuItem { std::string label; bool* val; };
    std::vector<MenuItem> items = {
        {"Desktop Notifications", &cfg.showNotifications},
        {"Auto-Shuffle on Start", &cfg.autoShuffle},
        {"Start Muted",           &cfg.startMuted},
        {"Show Visuals",          &cfg.showVisuals}
    };

    int totalItems = items.size() + 1; // Toggles + 1 for Quality

    // 1. Draw standard toggles
    for (int i = 0; i < items.size(); ++i) {
        buffer << "\033[" << (10 + i) << ";10H";
        if (i == selectedConfig) buffer << WHITE << " > " << BLUE;
        else buffer << "   ";

        buffer << items[i].label << ": ";

        // COLOR LOGIC FOR ON/OFF
        if (*(items[i].val)) {
            buffer << GREEN << "[ON]" << BLUE; // Green if true
        } else {
            buffer << RED << "[OFF]" << BLUE;  // Red if false
        }
    }

    // 2. Draw the Quality Selector row (High is usually Green, others Yellow/Red)
    int qIdx = items.size();
    buffer << "\033[" << (10 + qIdx) << ";10H";
    if (selectedConfig == qIdx) buffer << WHITE << " > " << BLUE;
    else buffer << "   ";

    buffer << "Audio Quality: [" << GREEN << cfg.quality << BLUE << "]";

    // 3. Add the "Note" if Highest is selected
    if (cfg.quality == "highest") {
        buffer << "\033[" << (1 + qIdx + 1) << ";10H"
        << "\033[93m" << "! Note: 'Highest' may delay or stall notifications and title updates." << BLUE;
    }
    if (cfg.showVisuals) {
        // 1. Start the buffer with the position
        buffer << "\033[" << (2 + qIdx + 1) << ";10H";

        #ifdef __HAIKU__
        // 2. Handle Haiku output
        buffer << "You will need to add milkdrop presets to $HOME/config/settings/MusicThingy/milk_presets/" << BLUE << "\n";
        #else
        // 3. Handle Linux output with the row skip
        buffer << "\033[93m" << "! Note:\n"
        << "\033[" << (2 + qIdx + 1) << ";10H"
        << "\033[93m" << "Use pavucontrol to switch recording to 'Monitor' for this to work.\n"
        << "\033[" << (3 + qIdx + 1) << ";10H" // Added one more skip so the "Also" isn't at column 1
        << "Also, you will need to add milkdrop presets to $HOME/.config/MusicThingy/milk_presets/" << BLUE << "\n";
        #endif
    }


    if (std::time(nullptr) < saveMessageTimer) {
        buffer << "\033[" << (w.ws_row - 3) << ";10H" << "Settings saved to: " << ORANGE << configPath << RESET;
    }


    buffer << get_ui_footer(w.ws_row);
    buffer << RESET;
    std::cout << buffer.str() << std::flush;

    if (kbhit()) {
        char c = std::tolower(getchar());
        if (c == 'b' || c == 27) return false;
        if (c == 'k' && selectedConfig > 0) selectedConfig--;
        if (c == 'j' && selectedConfig < totalItems - 1) selectedConfig++;


        // Handling the Enter Key to Toggle/Cycle
        if (c == '\n' || c == '\r') {
            if (selectedConfig < items.size()) {
                // 1. Toggle the boolean value
                *(items[selectedConfig].val) = !(*(items[selectedConfig].val));
                
#ifdef USE_PROJECTM
                // 2. NEW: Sync the Visualizer Window if "Show Visuals" was toggled
			if (items[selectedConfig].label == "Show Visuals") {
    			if (cfg.showVisuals) {
     			   if (!visualsRunning) init_visuals();
   			 } else {
 			       if (visualsRunning) {
     			       visualsRunning = false;
     		       if (glContext) { SDL_GL_DeleteContext(glContext); glContext = nullptr; }
     		       if (visualWin) { SDL_DestroyWindow(visualWin); visualWin = nullptr; }            
     		       // CLEAN WRAPPER CALLED HERE
      		      cleanup_capture_device(); 
    		   }
  			  }
			}
#endif
            } else {
                // Cycle the Quality string...
                if (cfg.quality == "highest") cfg.quality = "high";
                else if (cfg.quality == "high") cfg.quality = "low";
                else cfg.quality = "highest";
            }
            save_config(); // Save immediately to disk

            // --- START THE TIMER HERE ---
            saveMessageTimer = std::time(nullptr) + 3; // Show for 3 seconds
            needsRedraw = true;                        // Trigger a UI refresh
            
#ifdef USE_PROJECTM
           // Handle window events if visuals are active
            if (visualsRunning && pm != nullptr) {
                SDL_Event e;
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) {
                        visualsRunning = false;
                    }
                    // KEEP THIS: This updates projectM and OpenGL when you hit 'F' or drag the corner
                    else if (e.type == SDL_WINDOWEVENT) {
                        if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                            glViewport(0, 0, e.window.data1, e.window.data2);
                            projectm_set_window_size(pm, e.window.data1, e.window.data2);
                        }
                    }
                }              
            }
#endif 
            usleep(10000); // Keep the menu snappy
            return true;

            needsRedraw = true;
        }
        
    }
    return true;
}



#ifdef USE_PROJECTM
void update_visuals_logic() {
    if (!visualsRunning || !pm) return;

    uint32_t currentTime = SDL_GetTicks();

    // Check if 30 seconds have passed
    if (currentTime - lastPresetChange >= PRESET_DURATION) {
        load_random_preset(pm); // The function we wrote earlier
        lastPresetChange = currentTime;
    }
}
#endif

// --- Main Engine ---

int main(int argc, char* argv[]) {
    // 1. FUNDAMENTAL LOAD (Always first)
    load_config();
    srand(time(0));

    // 2. CLI SENDER LOGIC (Checks if we are just sending a command to a running instance)
    if (argc > 1) {
        std::string cmd = argv[1]; // Get the command (e.g., "shuffle")
        int fd = open(fifoPath, O_WRONLY | O_NONBLOCK);

        if (fd == -1) {
            std::cerr << "MusicThingy is not running." << std::endl;
            return 1;
        }
        // --- NEW: HELP COMMAND (Doesn't need the FIFO running) ---
        if (cmd == "help" || cmd == "--help" || cmd == "-h") {
            std::cout << "\n\033[1;36mMusic Thingy CLI Help\033[0m\n"
            << "--------------------------\n"
            << "Usage: MusicThingy [command]\n\n"
            << "Commands:\n"
            << "  \033[1;32mstatus\033[0m   - Show current song, volume, and visualizer preset\n"
            << "  \033[1;32mshuffle\033[0m  - Skip to the next song in the queue\n"
            << "  \033[1;32mvisual\033[0m   - Shuffle to a new random Milkdrop preset\n"
            << "  \033[1;32mtoggle\033[0m   - Play/Pause the music\n"
            << "  \033[1;32mquit\033[0m     - Close the running MusicThingy instance\n"
            << "--------------------------\n" << std::endl;
            return 0; // Exit help immediately
        }


        if (cmd == "status") {
            mkfifo(respPath, 0666);
            int respFd = open(respPath, O_RDONLY | O_NONBLOCK);
            write(fd, "status", 6);
            close(fd);

            // Wait for response
            for(int i = 0; i < 50; ++i) {
                char buf[512] = {0};
                if (read(respFd, buf, sizeof(buf)-1) > 0) {
                    std::cout << buf << std::endl;
                    close(respFd); unlink(respPath);
                    return 0;
                }
                usleep(10000);
            }
            close(respFd); unlink(respPath);
            return 1;
        }

        // For all other commands (shuffle, quit, etc.)
        write(fd, cmd.c_str(), cmd.length());
        close(fd);
        return 0; // EXIT SENDER IMMEDIATELY
    }

    // 3. TERMINAL WRAPPER (Ensures we are in a visible window)
    if (!isatty(STDIN_FILENO)) {
        std::string path = get_self_path();
        if (path.empty()) return 1;

        std::string cmd = "";

        #ifdef __HAIKU__
        // Haiku: 'Terminal' is always available.
        cmd = "Terminal -t \"SuperMusicThingy\" " + path + " &";
        #else
        // Linux: Search for available terminals
        struct Term { std::string name; std::string flag; };
        std::vector<Term> terms = {
            {"x-terminal-emulator", "-e"},
            {"konsole", "-e"},
            {"gnome-terminal", "--"}, // Modern GNOME requires '--' for command execution
            {"xfce4-terminal", "-e"},
            {"xterm", "-e"}
        };

        for (const auto& t : terms) {
            // Check if the terminal exists in the user's PATH
            if (system(("command -v " + t.name + " >/dev/null 2>&1").c_str()) == 0) {
                cmd = t.name + " " + t.flag + " \"" + path + "\" &";
                break;
            }
        }
        #endif

        if (!cmd.empty()) { system(cmd.c_str()); return 0; }
        return 1;
    }

    // 4. ACTUAL PLAYER INITIALIZATION (Runs only once)
    init_mpv();        // Start MPV engine
    fetch_channels();  // Load SomaFM list



#ifdef USE_PROJECTM
    if (cfg.showVisuals) {
        init_visuals(); // Open the eyes and ears
    }
#endif

    // OS Signals & FIFO
    signal(SIGWINCH, handle_resize);
    atexit(cleanup_fifo);
    mkfifo(fifoPath, 0666);
    int fifoFd = open(fifoPath, O_RDWR | O_NONBLOCK);

    // Auto-play
    if (cfg.autoShuffle) {
        play_random();
    }

    // UI Start
    system("stty raw -echo");
    draw_ui();

    // 5. THE MAIN LOOP
    while (true) {
        bool needsRedraw = false;

        // --- Inside while(true) loop ---

        // ONLY fire the notification here when the fuse burns down
        if (notifyTimer > 0 && std::time(nullptr) >= notifyTimer) {
            currentSong = pendingSong;

            // IMPORTANT: This is the ONLY place send_notification should be called
            if (cfg.showNotifications) {
                send_notification(currentStation, currentSong);
            }

            notifyTimer = 0; // Reset the fuse
            needsRedraw = true;
        }


        // A. FIFO LISTENER
        char cmdBuf[64]; // Buffer for incoming commands
        ssize_t bytes = read(fifoFd, cmdBuf, sizeof(cmdBuf) - 1);
        if (bytes > 0) {
            cmdBuf[bytes] = '\0';
            std::string cmd(cmdBuf);

            // --- Inside the FIFO Listener (Player side) ---
            if (cmd == "status") {
                int respFd = open(respPath, O_WRONLY | O_NONBLOCK);
                if (respFd != -1) {
                    std::stringstream ss;
                    ss << "\033[1;36m--- Music Thingy Status ---\033[0m\n"
                    << "Song:      " << currentSong << "\n"
                    << "Desc:      " << currentDesc << "\n"
                    << "Station:   " << currentStation << "\n"
                    << "Listeners: " << currentListeners << "\n"
                    << "Total Ch:  " << channels.size() << "\n"
                    << "Favorites: " << count_favorites() << "\n"
                    << "Quality:   " << get_bitrate_text() << "\n"
                    << "Volume:    " << get_vol_bar() << "\n";
                    #ifdef USE_PROJECTM
                    if (visualsRunning) {
                        ss << "Visual:    " << currentPresetName << "\n";
                    }
                    #endif

                   ss << "\033[1;36m---------------------------\033[0m";

                    std::string reply = ss.str();
                    write(respFd, reply.c_str(), reply.length());
                    close(respFd);
                }
            }


            else if (cmd == "favorites") {
                play_favorite(); // Reuse existing play_favorite() random logic
                needsRedraw = true;
            }
            else if (cmd == "add_fav") {
                save_favorite();
            }
            else if (cmd == "del_fav") {
                delete_favorite();
            }
            else if (cmd == "quit") {
                goto end; // Jumps to your cleanup and exit logic
            }
            else if (cmd == "shuffle") {
                play_random();
                needsRedraw = true;
            }
            else if (cmd == "vol_up") {
                set_volume('+');
                needsRedraw = true;
            }
            else if (cmd == "vol_down") {
                set_volume('-');
                needsRedraw = true;
            }

        }

        // B. MENU SCREENS

        if (showConfig) {
            showConfig = draw_config_menu(); // Update state directly from the function
            if (!showConfig) {
                draw_ui();
            }
            usleep(10000);
            continue;
        }

        if (showHelp) {
            showHelp = draw_help_menu(); // Update state directly from the function
            if (!showHelp) {
                draw_ui();
            }
            usleep(10000);
            continue;
        }

        if (showMenu) {
            // Apply the same logic to your favorites menu
            showMenu = draw_favorites_menu();
            if (!showMenu) {
                draw_ui();
            }
            usleep(10000);
            continue;
        }

        // C. STATUS EXPIRY

        // Auto-clear status message after timeout
        if (!statusMsg.empty() && std::time(nullptr) >= statusExpiry) {
            statusMsg = "";
            needsRedraw = true;
        }
        // Check if terminal was resized
        if (resized) {
            resized = 0;
            needsRedraw = true;
        }

        // D. MPV EVENTS (Buffered + Delayed Notification Version)
        while (true) {
            mpv_event *event = mpv_wait_event(mpv, 0);
            if (event->event_id == MPV_EVENT_NONE) break;

            if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
                mpv_event_property *prop = (mpv_event_property *)event->data;
                if (prop && prop->data) {
                    std::string propName = prop->name;

                    // 1. Handle Title Changes
                    if (propName == "media-title") {
                        char* title_ptr = *(char **)prop->data;
                        if (title_ptr) {
                            std::string newTitle = title_ptr;
                            if (newTitle.find("http") != 0 && newTitle != currentSong) {
                                // RE-ENABLE THE FUSE:
                                pendingSong = newTitle;
                                notifyTimer = std::time(nullptr) + 2;
                            }
                        }
                    }
                    // 2. Handle Buffering Safety
                    else if (propName == "paused-for-cache") {
                        int is_buffering = *(int *)prop->data;
                        if (!is_buffering) {
                            char* t = mpv_get_property_string(mpv, "media-title");
                            if (t && std::string(t).find("http") != 0 && std::string(t) != currentSong) {
                                // RE-ENABLE THE FUSE HERE TOO:
                                pendingSong = t;
                                notifyTimer = std::time(nullptr) + 2;
                            }
                            if (t) mpv_free(t);
                        }
                    }
                }
            }
            if (event->event_id == MPV_EVENT_SHUTDOWN) goto end;
        }


        // E. KEYBOARD INPUT
        if (kbhit()) {
            char input = getchar(); // Get the raw key once
            char c = std::tolower(input); // Create a lowercase version for the switch

            if (c == 'q') break; // 'Q' and 'q' both quit

            switch (c) {
                case 'l': showMenu = true; selectedFav = 0; break;
                case 's': play_random(); currentSong = "Buffering..."; break;
                case 'a': save_favorite(); break;
                case 'c': showConfig = true; break;
                case 'f': play_favorite(); break;
                case 'd': delete_favorite(); break;

			#ifdef USE_PROJECTM
                case 'v':
                        load_random_preset(pm);
                        lastPresetChange = SDL_GetTicks();
                        break;
            #endif
                case 'h': showHelp = true; break;
                case 'n': play_random(); break; // Added your 'N' key!
                case '+': case '-':
                    set_volume(input); // Use 'input' here so '+' works correctly
                    break;
                case 'm': toggle_mute(); break;

			#ifdef USE_PROJECTM
               		case 'k': {
                    // 1. Get current window flags
                    uint32_t flags = SDL_GetWindowFlags(visualWin);
                    bool isFullscreen = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP);

                    // 2. Toggle state: if fullscreen, go windowed (0); if windowed, go fullscreen
                    SDL_SetWindowFullscreen(visualWin, isFullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);

                    // 3. Immediately update projectM with new dimensions
                
                    int w, h;
                    SDL_GetWindowSize(visualWin, &w, &h);
                    glViewport(0, 0, w, h);
                    projectm_set_window_size(pm, w, h);
                    break;
               
                }
             #endif
            }
            needsRedraw = true;
        }



        if (needsRedraw || resized) {
            resized = 0;
            draw_ui();
        }



#ifdef USE_PROJECTM
        // F. --- VISUALS LOGIC ---
        if (visualsRunning && pm) {
            #ifdef __HAIKU__
            if (alcCaptureDevice) {
                ALCint samples = 0;
                // Check how many samples are ready in the OpenAL buffer
                alcGetIntegerv(alcCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &samples);

                if (samples > 1024) {
                    short buffer[2048]; // Stereo 16-bit
                    alcCaptureSamples(alcCaptureDevice, (ALCvoid*)buffer, 1024);

                    // Convert Short (Int16) to Float for projectM
                    float floatBuffer[2048];
                    for (int i = 0; i < 2048; ++i) {
                        floatBuffer[i] = buffer[i] / 32768.0f;
                    }
                    projectm_pcm_add_float(pm, floatBuffer, 1024, PROJECTM_STEREO);
                }
            }
            #else

            // --- LINUX/SDL PUMP ---
            uint32_t queued = SDL_GetQueuedAudioSize(captureDevice);
            if (queued >= sizeof(audioBuffer)) {
                SDL_DequeueAudio(captureDevice, audioBuffer, sizeof(audioBuffer));
                // projectM-4 uses projectm_pcm_add_float for SDL's F32 format
                projectm_pcm_add_float(pm, audioBuffer, 1024, PROJECTM_STEREO);
            }
            #endif

            // Render and Swap
            projectm_opengl_render_frame(pm);
            SDL_GL_SwapWindow(visualWin);

            // Handle window events (closing the visualizer window)
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) visualsRunning = false;
            }
        }  
        #endif
        usleep(16000);     
    }



    end:
    system("stty cooked echo");

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    std::stringstream buffer;
    buffer << get_ui_footer(w.ws_row) << BLUE << "Good bye! " << RESET << std::endl;
    std::cout << buffer.str();

    if (mpv) mpv_terminate_destroy(mpv);
    return 0;
}
