/* SuperMusicThingy
 *
 * Copyright (c) 2026 ablyss
 * See the The MIT License included in this folder
 *
 * Some AI was used to help make this possibe.
 * Original inspiration started with a bash script
 *
 */



#include <algorithm>
#include <curl/curl.h>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <mpv/client.h>
#include <mutex>
#include "nlohmann/json.hpp"
#include <poll.h>
#include <random>
#include <unistd.h>
#include <set>
#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <vector>

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

#ifdef __LINUX__
#include <libnotify/notify.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

// Icon
static const unsigned char icon_24px_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xe0, 0x77, 0x3d, 0xf8, 0x00, 0x00, 0x02,
    0xde, 0x49, 0x44, 0x41, 0x54, 0x48, 0x89, 0xd5, 0x94, 0x3f, 0x48, 0x1b,
    0x51, 0x1c, 0xc7, 0x3f, 0xf9, 0x83, 0x9c, 0x20, 0x94, 0x54, 0xa2, 0x41,
    0xeb, 0x9f, 0xe3, 0x30, 0x38, 0x54, 0xc1, 0x25, 0x51, 0xcf, 0x58, 0x34,
    0x60, 0xe9, 0xd0, 0xa5, 0x74, 0x2d, 0x38, 0x94, 0xd6, 0xa5, 0x4b, 0xf7,
    0xe2, 0xd0, 0xbd, 0x94, 0x0e, 0x05, 0x3b, 0x74, 0x11, 0xba, 0x74, 0x75,
    0x68, 0xa1, 0xd4, 0xc5, 0xa8, 0x31, 0x12, 0x82, 0xb4, 0x36, 0x14, 0x5a,
    0x93, 0xe6, 0x4c, 0x08, 0xd2, 0x0a, 0x25, 0x68, 0xe2, 0xdd, 0xe5, 0x75,
    0xc8, 0x85, 0x5e, 0xce, 0x06, 0x95, 0xa6, 0x43, 0xbf, 0xf0, 0x38, 0xee,
    0xbd, 0xef, 0xfb, 0x7d, 0x7f, 0xbf, 0xf7, 0xde, 0xef, 0x0b, 0xff, 0x01,
    0x96, 0x80, 0x37, 0x80, 0xff, 0x5f, 0x09, 0x88, 0xe9, 0xe9, 0x69, 0x01,
    0x7c, 0x05, 0xae, 0x38, 0x17, 0xdd, 0xad, 0x50, 0x88, 0x44, 0x22, 0x4c,
    0x4d, 0x4d, 0xc9, 0xc0, 0x9a, 0x53, 0xa4, 0x2e, 0x10, 0x04, 0x56, 0x00,
    0xe1, 0x18, 0xdf, 0x80, 0x45, 0xc0, 0x67, 0x8d, 0x45, 0x6b, 0xce, 0xce,
    0xc1, 0x30, 0x0c, 0x54, 0x55, 0x65, 0x72, 0x72, 0x72, 0xc0, 0x29, 0xe2,
    0xb2, 0xbe, 0x6f, 0xc3, 0xe1, 0xf0, 0x9c, 0xaa, 0xaa, 0x0d, 0x99, 0x1d,
    0x1e, 0x1e, 0xb2, 0xb1, 0xb1, 0x41, 0x3a, 0x9d, 0xde, 0x02, 0x4a, 0xc3,
    0xc3, 0xc3, 0xb3, 0x13, 0x13, 0x13, 0xf8, 0x7c, 0xbe, 0xa6, 0xd5, 0xac,
    0xaf, 0xaf, 0xb3, 0xb9, 0xb9, 0xb9, 0x01, 0xcc, 0x02, 0x65, 0x6f, 0x7d,
    0xa1, 0x5a, 0xad, 0xa2, 0xeb, 0x7a, 0x03, 0xb9, 0xa3, 0xa3, 0x83, 0x68,
    0x34, 0xca, 0xfe, 0xfe, 0x7e, 0x48, 0x92, 0x24, 0xa2, 0xd1, 0x28, 0x6e,
    0xb7, 0xfb, 0x14, 0xcf, 0x0e, 0xd3, 0x34, 0x01, 0xda, 0x80, 0x29, 0xe0,
    0x5d, 0x5d, 0xe0, 0x41, 0x22, 0x91, 0x58, 0x4e, 0x24, 0x12, 0x21, 0x3b,
    0x59, 0x55, 0x55, 0x46, 0x46, 0x46, 0x70, 0xbb, 0xdd, 0x8c, 0x8f, 0x8f,
    0x63, 0x9a, 0x26, 0xa9, 0x54, 0x8a, 0x58, 0x2c, 0xd6, 0x10, 0x74, 0x61,
    0x61, 0x01, 0x80, 0xad, 0xad, 0x2d, 0x92, 0xc9, 0x64, 0x16, 0x78, 0x55,
    0x5f, 0xab, 0x0b, 0x7c, 0x06, 0xe6, 0x80, 0x09, 0xa0, 0xdf, 0x9a, 0xeb,
    0x8c, 0xc7, 0xe3, 0x8f, 0x03, 0x81, 0x80, 0xa7, 0x52, 0xa9, 0xe0, 0xf7,
    0xfb, 0x29, 0x97, 0xcb, 0xc4, 0xe3, 0x71, 0x13, 0x78, 0x04, 0x7c, 0xb7,
    0x78, 0x4b, 0x86, 0x61, 0xb0, 0xbd, 0xbd, 0x4d, 0x2a, 0x95, 0xca, 0x01,
    0xcf, 0xa8, 0xdd, 0xd3, 0x5a, 0xd3, 0x32, 0x6d, 0x58, 0x55, 0x14, 0x45,
    0x04, 0x83, 0x41, 0x31, 0x3f, 0x3f, 0x2f, 0x66, 0x66, 0x66, 0x04, 0x90,
    0x06, 0xee, 0x02, 0x1d, 0x16, 0x47, 0x8c, 0x8e, 0x8e, 0x0a, 0x40, 0x03,
    0x1e, 0x02, 0xb7, 0x01, 0xa9, 0x1e, 0xe0, 0xac, 0x67, 0xfa, 0x5a, 0xd3,
    0x34, 0x7a, 0x7b, 0x7b, 0xd1, 0x75, 0x9d, 0x4c, 0x26, 0x03, 0xf0, 0x05,
    0x28, 0x00, 0x25, 0x8b, 0xf3, 0x62, 0x67, 0x67, 0xe7, 0x23, 0xf0, 0xc4,
    0xca, 0x7c, 0x05, 0x28, 0x3b, 0x8f, 0xa8, 0x19, 0x62, 0xd5, 0x6a, 0x55,
    0x74, 0x76, 0x76, 0xba, 0x2a, 0x95, 0x0a, 0x9a, 0xa6, 0x09, 0xe0, 0x27,
    0xb0, 0x67, 0xe3, 0xdc, 0x07, 0x22, 0x56, 0x45, 0xab, 0xf6, 0xe0, 0xe7,
    0x11, 0x50, 0x03, 0x81, 0x80, 0xcb, 0x34, 0x4d, 0x8a, 0xc5, 0x22, 0x2e,
    0x97, 0xcb, 0x05, 0xc8, 0x40, 0xd1, 0xc6, 0x59, 0x02, 0x06, 0x80, 0x3b,
    0xce, 0xe0, 0x70, 0xf6, 0x11, 0xdd, 0xea, 0xee, 0xee, 0xc6, 0x30, 0x0c,
    0x34, 0x4d, 0xa3, 0xab, 0xab, 0x0b, 0xa0, 0x07, 0xe8, 0xb6, 0x71, 0xee,
    0x0d, 0x0d, 0x0d, 0x5d, 0x07, 0xe2, 0x5c, 0xd0, 0x2a, 0x06, 0x3d, 0x1e,
    0xcf, 0x35, 0x9f, 0xcf, 0xc7, 0xc9, 0xc9, 0x09, 0x85, 0x42, 0x81, 0xc1,
    0xc1, 0x41, 0x3c, 0x1e, 0x4f, 0x2f, 0x10, 0xb6, 0x13, 0x15, 0x45, 0x41,
    0x51, 0x94, 0x0b, 0x5b, 0xc5, 0x9e, 0xa2, 0x28, 0x5e, 0x21, 0x04, 0x07,
    0x07, 0x07, 0x78, 0xbd, 0x5e, 0x24, 0x49, 0x42, 0x51, 0x14, 0x37, 0xf0,
    0x12, 0x87, 0x55, 0xc8, 0xb2, 0x8c, 0x2c, 0xcb, 0xcd, 0xad, 0xa2, 0xbf,
    0xbf, 0x7f, 0x4e, 0x96, 0xe5, 0x53, 0x65, 0x08, 0x21, 0x48, 0x26, 0x93,
    0xe8, 0xba, 0x4e, 0x38, 0x1c, 0xa6, 0x76, 0x0d, 0xcd, 0x91, 0xc9, 0x64,
    0xc8, 0x66, 0xb3, 0xe7, 0xb3, 0x8a, 0xe3, 0xe3, 0x63, 0x72, 0xb9, 0x1c,
    0xa5, 0x52, 0xe9, 0x13, 0xd0, 0xb6, 0xbb, 0xbb, 0xab, 0xf4, 0xf5, 0xf5,
    0xd1, 0xde, 0xde, 0xde, 0x54, 0xc0, 0x69, 0x15, 0xf5, 0x74, 0x82, 0xc0,
    0x32, 0x10, 0x72, 0xf0, 0x7f, 0x00, 0x31, 0xe0, 0x3d, 0xb5, 0xb7, 0x7f,
    0x13, 0xb8, 0x01, 0x5c, 0xb6, 0x93, 0x42, 0xa1, 0xda, 0x36, 0x4d, 0xd3,
    0xc8, 0xe7, 0xf3, 0x59, 0x6a, 0xdd, 0xbc, 0x63, 0x17, 0x00, 0xb8, 0x44,
    0xa3, 0x55, 0xd4, 0x51, 0xa2, 0xd6, 0xbd, 0x1f, 0xac, 0xff, 0xab, 0xc0,
    0x30, 0xbf, 0x3b, 0x79, 0x69, 0x6c, 0x6c, 0x8c, 0x7c, 0x3e, 0x4f, 0xb1,
    0x58, 0xcc, 0x01, 0x4f, 0xf9, 0x43, 0xc3, 0xfd, 0x0d, 0x84, 0xdf, 0xef,
    0x6f, 0x6a, 0x15, 0x9e, 0x16, 0x08, 0xf4, 0x1c, 0x1d, 0x1d, 0x49, 0xc0,
    0x73, 0x5a, 0x9c, 0xb9, 0x1d, 0x11, 0x6a, 0x77, 0x23, 0x9d, 0x45, 0x6c,
    0x39, 0x7e, 0x01, 0x5f, 0xd9, 0x1b, 0xa3, 0x9b, 0x0b, 0xbc, 0x4f, 0x00,
    0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};




namespace fs = std::filesystem;

// Preset Shuffle
const uint32_t PRESET_DURATION = 30000;  // 30 Sec
uint32_t lastPresetChange = 0;
std::string currentPresetName = "None";
void update_visuals_logic();


#ifdef __HAIKU__
#include <image.h>
#include <OS.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <Application.h>
ALCdevice *alcCaptureDevice = nullptr;
#elif defined(USE_SDL2)
SDL_AudioDeviceID captureDevice = 0;
#endif

#if defined(__linux__)
int global_socket_fd = -1;

bool is_already_running_linux() {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return false;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;

    const char* socket_name = "SuperMusicThingyLock";
    addr.sun_path[0] = '\0';
    memcpy(&addr.sun_path[1], socket_name, strlen(socket_name));
    socklen_t addr_len = sizeof(addr.sun_family) + 1 + strlen(socket_name);

    if (bind(fd, (struct sockaddr*)&addr, addr_len) < 0) {
        close(fd);
        return true;
    }
    global_socket_fd = fd;
    return false;
}
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


void ensure_config_dir() {
    std::string path;

    #ifdef __HAIKU__
    path = "/boot/home/config/settings/SuperMusicThingy";
    #else
    const char* home = getenv("HOME");
    if (home) {
        path = std::string(home) + "/.config/SuperMusicThingy";
    } else {
        path = "./config";
    }
    #endif

    try {
        if (!fs::exists(path)) {

            if (fs::create_directories(path)) {
                std::cout << "Created config directory: " << path << std::endl;
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
    }
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
const std::string SCLR = "\033[H\033[J";
const std::string CLEARALL = "\033[2J\033[3J\033[H";
const std::string BGTRUEBLK = "\033[48;2;0;0;0m";
const std::string BLUE   = "\033[94m";
const std::string RED    = "\033[91m";
const std::string ORANGE = "\033[93m";
const std::string WHITE  = "\033[97m";
const std::string YELLOW = "\033[33m";
const std::string GREEN  = "\033[92m";
const std::string BLACK  = "\033[2J\033[3J\033[H";
const std::string niceGreenColor = "\033[92m";
const std::string RESET  = "\033[0m";

enum MenuState { NONE, FAVORITES, HELP, CONFIG };
MenuState currentMenu = NONE;


bool is_native_tty() {
    const char* term = std::getenv("TERM");
    return (term && std::string(term) == "linux");
}
std::string get_ui_header(int rows) {

    std::stringstream header;


    if (is_native_tty()) {
        // Safe mode for TTY2 (Ctrl+Alt+F2)
            header << SCLR;
    } else {
        // High-end mode for xterm/desktop
        header << BGTRUEBLK << BLUE << CLEARALL;
    }

    header << "\033[1;32H" << BLUE << "SuperMusicThingy\n";
    if (currentMenu == NONE) {
        header << "\033[2;20H" << "[" << ORANGE << "S" << BLUE << "]huffle | Vol [" << ORANGE << "+/-" << BLUE << "] | [" << ORANGE << "H" << BLUE << "]elp | [" << ORANGE << "Q" << BLUE << "]uit\n";
    }
    if (currentMenu == HELP) {
        header << "\033[2;20H" << "[" << ORANGE << "S" << BLUE << "]huffle | Vol [" << ORANGE << "+/-" << BLUE << "] | [" << ORANGE << "H" << BLUE << "]elp | [" << ORANGE << "B" << BLUE << "]ack\n";
    }
    if (currentMenu == FAVORITES) {
        header << "\033[2;22H" << "[" << ORANGE << "j/k" << BLUE << "] Scroll | [" << ORANGE << "Enter" << BLUE << "] Play | [" << ORANGE << "B" << BLUE << "]ack\n";
    }
    if (currentMenu == CONFIG) {
        header << "\033[2;21H" << "[" << ORANGE << "j/k" << BLUE << "] Scroll | [" << ORANGE << "Enter" << BLUE << "] Update | [" << ORANGE << "B" << BLUE << "]ack\n";
    }
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
bool showFavorites = false;
bool showHelp = false;
bool showNotifications = false;
bool showConfig = false;

// Path for the config file
#ifdef __HAIKU__
std::string configPath = getenv("HOME") + std::string("/config/settings/SuperMusicThingy/config.txt");
#else
std::string configPath = getenv("HOME") + std::string("/.config/SuperMusicThingy/config.txt");
#endif


struct Config {
    bool showNotifications = true;
    bool showVisuals = false;
    bool autoShuffle = false;
    bool autoShuffleVisuals = false;
    int defaultVolume = 100;
    std::string quality = "highest";
} cfg;

int selectedConfig = 0;


void save_config() {
    json j;
    j["quality"] = cfg.quality;
    j["showNotifications"] = cfg.showNotifications;
    j["autoShuffle"] = cfg.autoShuffle;
    j["autoShuffleVisuals"] = cfg.autoShuffleVisuals;
    j["showVisuals"] = cfg.showVisuals;

    std::ofstream outfile(configPath);
    outfile << j.dump(4);
}

void load_config() {
    std::ifstream infile(configPath);
    if (infile.is_open()) {
        try {
            json j = json::parse(infile);
            cfg.quality = j.value("quality", "high");
            cfg.showNotifications = j.value("showNotifications", true);
            cfg.autoShuffle = j.value("autoShuffle", false);
            cfg.autoShuffleVisuals = j.value("autoShuffleVisuals", false);
            cfg.showVisuals = j.value("showVisuals", true);
        } catch(...) {}
    }
}


#ifdef __HAIKU__
  #ifdef USE_PROJECTM
  // --- For reading arguments from keyboard shortcuts ---
  const char* fifoPath = "/tmp/SuperMusicThingyNebula_fifo";
  const char* respPath = "/tmp/SuperMusicThingyNebula_resp";
  int fifoFd = -1;
  #endif
#else
    const char* fifoPath = "/tmp/SuperMusicThingy_fifo";
    const char* respPath = "/tmp/SuperMusicThingy_resp";
    int fifoFd = -1;
#endif



// Delete fifo on exit
void cleanup_fifo() {
    unlink(fifoPath);
    unlink(respPath);
}

// Signal handler wrapper
void handle_exit_signal(int sig) {
    cleanup_fifo();
    _exit(0); // Exit immediately after cleanup
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
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "SuperMusicThingy/1.0");
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
    std::string configPath = std::string(home) + "/config/settings/SuperMusicThingy/milk_presets/";
    #else
    std::string configPath = std::string(home) + "/.config/SuperMusicThingy/milk_presets/";
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
        std::string name = std::filesystem::path(selected).stem().string();
        if (name.length() > 46) {
            currentPresetName = name.substr(0, 43) + "...";
        } else {
            currentPresetName = name;
        }

        needsRedraw = true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "FS Error: " << e.what() << std::endl;
    }
}
#endif


#ifdef USE_PROJECTM
void init_visuals() {
    if (visualWin) return;
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
        visualWin = SDL_CreateWindow("SuperMusicThingy Visuals",
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
        if (cfg.quality == "highest") {
            return BASE_URL + id + ".pls";
        }

        if (cfg.quality == "high") {
            return BASE_URL + id + "64.pls";  
        }

        if (cfg.quality == "low") {
            return BASE_URL + id + "32.pls";  
        }

        // Default: 128k AAC (id + "130.pls")
        return BASE_URL + id + ".pls";
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
        std::ifstream infile(home + "/config/settings/SuperMusicThingy/favorites.txt");
        #else
        std::ifstream infile(home + "/.config/SuperMusicThingy/favorites.txt");
        #endif
        int lines = 0;
        std::string line;
        while (std::getline(infile, line)) if (!line.empty()) lines++;
        return lines;
    }

    bool is_favorite() {
        std::string home = getenv("HOME") ? getenv("HOME") : ".";
        #ifdef __HAIKU__
        std::ifstream infile(home + "/config/settings/SuperMusicThingy/favorites.txt");
        #else
        std::ifstream infile(home + "/.config/SuperMusicThingy/favorites.txt");
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
    
    // Volume bar
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
       

    bool draw_config_menu() {
        struct winsize w; ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        std::stringstream buffer;
        buffer << get_ui_header(w.ws_row);

        ensure_config_dir();


        // Define the list of options to display
        struct MenuItem { std::string label; bool* val; };
        std::vector<MenuItem> items = {
            {"Desktop Notifications", &cfg.showNotifications},
            {"Auto-Shuffle on Start", &cfg.autoShuffle},
            {"Auto-Shuffle Visuals / 30s", &cfg.autoShuffleVisuals},
            {"Show Visuals", &cfg.showVisuals}
        };

        int totalItems = items.size() + 1; // Toggles + 1 for Quality

        // 1. Draw standard toggles
        for (int i = 0; i < items.size(); ++i) {
            buffer << "\033[" << (10 + i) << ";10H";
            if (i == selectedConfig) buffer << ORANGE << " > " << BLUE;
            else buffer << "   ";

            buffer << items[i].label << ": ";

            // COLOR LOGIC FOR ON/OFF
            if (*(items[i].val)) {
                buffer << GREEN << "[ON]" << BLUE;
            } else {
                buffer << RED << "[OFF]" << BLUE;
            }
        }

        // 2. Draw the Quality
        int qIdx = items.size();
        buffer << "\033[" << (10 + qIdx) << ";10H";
        if (selectedConfig == qIdx) buffer << ORANGE << " > " << BLUE;
        else buffer << "   ";

        buffer << "Audio Quality: [" << GREEN << cfg.quality << BLUE << "]";

        if (std::time(nullptr) < saveMessageTimer) {
            buffer  << "\033[" << w.ws_row << ";23H" << ORANGE << "Settings saved." << ORANGE;
        }

        buffer << get_ui_footer(w.ws_row);
        buffer << RESET;
        std::cout << buffer.str() << std::flush;

        if (kbhit()) {
            char c = std::tolower(getchar());

            // Global keys
            if (c == 's') { play_random(); currentSong = "Buffering...";  return false; }
            if (c == '+') { set_volume('+'); return false; }
            if (c == '-') { set_volume('-'); return false; }
            if (c == 'c') { showConfig = true; currentMenu = CONFIG; return false; }
            if (c == 'l') { showFavorites = true; selectedFav = 0; currentMenu = FAVORITES; return false; }
            if (c == 'h') { showHelp = true; currentMenu = HELP; return false; }


            if (c == 'b' || c == 27) {
                currentMenu = NONE;
                return false;

            }
            if (c == 'j' && selectedConfig > 0) selectedConfig--;
            if (c == 'k' && selectedConfig < totalItems - 1) selectedConfig++;


            // Handling the Enter Key to Toggle/Cycle
            if (c == '\n' || c == '\r') {
                if (selectedConfig < items.size()) {
                    // 1. Toggle the boolean value
                    *(items[selectedConfig].val) = !(*(items[selectedConfig].val));

                    #ifdef USE_PROJECTM
                    // 2. NEW: Sync the Visualizer Window if "Show Visuals" was toggled
                    if (items[selectedConfig].label == "Show Visuals") {
                        if (cfg.showVisuals) {
                            if (!visualsRunning and !is_native_tty()) init_visuals();
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
                saveMessageTimer = std::time(nullptr) + 3;
                needsRedraw = true;

                #ifdef USE_PROJECTM
                // Handle window events if visuals are active
                if (visualsRunning && pm != nullptr) {
                    SDL_Event e;
                    while (SDL_PollEvent(&e)) {
                        if (e.type == SDL_QUIT) {
                            visualsRunning = false;
                        }
                        // KEEP THIS: This updates projectM and OpenGL when you hit 'k' or drag the corner
                        else if (e.type == SDL_WINDOWEVENT) {
                            if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                                glViewport(0, 0, e.window.data1, e.window.data2);
                                projectm_set_window_size(pm, e.window.data1, e.window.data2);
                            }
                        }
                    }
                }
                #endif
                usleep(33333); 
                return true;

                needsRedraw = true;
            }

        }

        return true;
    }


    bool draw_help_menu() {
        struct winsize w; ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        std::stringstream buffer;

        buffer << get_ui_header(w.ws_row);
        buffer << "\033[5;33H" <<  ORANGE << "--- HELP ---" << BLUE;

        int r = 7;
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "s/n" << BLUE << "] Shuffle    : Play a random station";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "f" << BLUE << "] Play Fav     : Play a random favorite";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "l" << BLUE << "] List Favs    : Open scrollable favorite menu";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "a" << BLUE << "] Add Fav      : Save current station to favorites list";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "d" << BLUE << "] Delete Fav   : Remove current station from favorites list";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "+/-" << BLUE << "] Volume     : Increase/Decrease volume";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "m" << BLUE << "] Mute         : Toggle audio mute";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "j/k" << BLUE << "] Scroll     : Scroll up/down selection";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "enter" << BLUE << "] Play     : Play or Update selection";
        #ifdef USE_PROJECTM
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "k" << BLUE << "] Fullscreen   : Fullscreen visual effects window";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "v" << BLUE << "] Shuffle      : Shuffle milk drop presets";
        #endif
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "x" << BLUE << "] Stop         : Stop the music";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "p" << BLUE << "] Toggle       : Play/Pause the music";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "h" << BLUE << "] Help         : Show this menu";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "b" << BLUE << "] Back         : Return to Main Menu";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "c" << BLUE << "] Config       : Config Manager";
        buffer << "\033[" << r++ << ";17H [" << ORANGE << "q" << BLUE << "] Quit         : Exit Music Thingy";
        buffer << "\033[" << r++ << ";17H ";

        #ifndef __HAIKU__
        buffer << "\033[" << r++ << ";17H" << ORANGE << "* " << BLUE << "Visuals: Set pavucontrol to switch recording to 'Monitor'";
        buffer << "\033[" << r++ << ";17H";
        buffer << "\033[" << r++ << ";17H"  << ORANGE << "* " << BLUE << "Milkdrop presets:";
        buffer << "\033[" << r++ << ";17H \n$HOME/.config/SuperMusicThingy/milk_presets/";
        #else
        buffer << "\033[" << r++ << ";17H"  << ORANGE << "*" << BLUE << " Milkdrop presets:";
        buffer << "\033[" << r++ << ";17H \n$HOME/config/settings/SuperMusicThingy/milk_presets/";
        #endif

        buffer << get_ui_footer(w.ws_row);
        buffer << RESET;
        std::cout << buffer.str() << std::flush;
        needsRedraw = false;


        if (kbhit()) {
            char c = getchar();
            // Global Keus
            if (c == 's') { play_random(); currentSong = "Buffering...";  return false; }
            if (c == '+') {  set_volume('+'); return false; }
            if (c == '-') {  set_volume('-'); return false; }
            if (c == 'c') { showConfig = true; currentMenu = CONFIG; return false; }
            if (c == 'l') { showFavorites = true; selectedFav = 0; currentMenu = FAVORITES; return false; }
            if (c == 'h') { showHelp = true; currentMenu = HELP; return false; }
            if (c == 'b' || c == 27 || c == 'h') {
                currentMenu = NONE;
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
        std::ifstream infile(home + "/config/settings/SuperMusicThingy/favorites.txt");
        #else
        std::ifstream infile(home + "/.config/SuperMusicThingy/favorites.txt");
        #endif

        std::vector<std::string> favUrls;
        std::string line;
        while (std::getline(infile, line)) if (!line.empty()) favUrls.push_back(line);

        // 2. Build UI

        buffer << get_ui_header(w.ws_row);
        buffer << "\033[5;30H" <<  ORANGE << "--- FAVORITES ---" << BLUE;
        int maxVisible = 10;
        if (favUrls.empty()) {
            buffer << "\033[7;25H  (No favorites saved yet)";
        } else {
            if (selectedFav < scrollOffset) scrollOffset = selectedFav;
            if (selectedFav >= scrollOffset + maxVisible) scrollOffset = selectedFav - maxVisible + 1;

            for (int i = 0; i < maxVisible && (i + scrollOffset) < (int)favUrls.size(); ++i) {
                int idx = i + scrollOffset;
                buffer << "\033[" << (7 + i) << ";19H";
                if (idx == selectedFav) buffer << ORANGE << " > " <<  ORANGE << favUrls[idx] << BLUE;
                else buffer << "   " << favUrls[idx];
            }
        }

        buffer << get_ui_footer(w.ws_row);
        buffer << RESET;
        std::cout << buffer.str() << std::flush;




        // 3. Handle Input
        if (kbhit()) {
            char c = getchar();
            // Global Keys
            // Global keys

            if (c == 's') { play_random(); currentSong = "Buffering...";  return false; }
            if (c == '+') { set_volume('+'); return false; }
            if (c == '-') { set_volume('-'); return false; }
            if (c == 'c') { showConfig = true; currentMenu = CONFIG; return false; }
            if (c == 'l') { showFavorites = true; selectedFav = 0; currentMenu = FAVORITES; return false; }
            if (c == 'h') { showHelp = true; currentMenu = HELP; return false; }

            if (c == 'b' || c == 27) {
                currentMenu = NONE;
                return false; // Exit menu
            }
            if (c == 'j' && selectedFav > 0) selectedFav--;
            if (c == 'k' && selectedFav < (int)favUrls.size() - 1) selectedFav++;

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
                currentMenu = NONE;

                return false; // Exit menu after playing
            }
        }

        return true; // Keep menu open if no exit key was pressed
    }

    // Rainbow Text template
    //[\033[31mF\033[33ma\033[32mv\033[36mo\033[34m\033[35mr\033[31mi\033[33mt\033[32me\033[94m]

    std::string get_bitrate_text() {
        if (cfg.quality == "highest") return "128k";
        if (cfg.quality == "low")     return "32k";
        return "64k"; // Default for "high"
    }


    // Wrapper functons
    int draw_wrapped_currentSong(std::stringstream& ss, const std::string& text, int termWidth, int startRow) {
        if (text.empty() || text == "None") return 0;

        std::stringstream words(text);
        std::string word;
        std::string currentLine = "";
        int linesUsed = 1;

        int maxLineWidth = termWidth - 13 - 2; // -2 for a small right-side margin

        // 1.
        ss << "\033[" << startRow << ";10H" << BLUE << " * Title: " << GREEN;

        bool firstLine = true;
        while (words >> word) {
            // Calculate available space on the CURRENT line
            int availableSpace = firstLine ? (maxLineWidth - 13) : maxLineWidth;

            if (currentLine.length() + word.length() + 1 <= (size_t)availableSpace) {
                if (!currentLine.empty()) currentLine += " ";
                currentLine += word;
            } else {
                // Print what we have
                ss << currentLine;
                linesUsed++;
                currentLine = word;
                firstLine = false;               
                ss << "\033[" << (startRow + linesUsed - 1) << ";13H" << GREEN;
            }
        }
     
        ss << currentLine << BGTRUEBLK;
        return linesUsed;
    }


    int draw_wrapped_description(std::stringstream& ss, const std::string& text, int termWidth, int startRow) {
        if (text.empty() || text == "None") return 0;

        std::stringstream words(text);
        std::string word;
        std::string currentLine = "";
        int linesUsed = 1;

        int maxLineWidth = termWidth - 13 - 2; 

        ss << "\033[" << startRow << ";10H" << BLUE << " * Description: " << GREEN;

        bool firstLine = true;
        while (words >> word) {
   
            int availableSpace = firstLine ? (maxLineWidth - 13) : maxLineWidth;

            if (currentLine.length() + word.length() + 1 <= (size_t)availableSpace) {
                if (!currentLine.empty()) currentLine += " ";
                currentLine += word;
            } else {
  
                ss << currentLine;
                linesUsed++;
                currentLine = word;
                firstLine = false;

                ss << "\033[" << (startRow + linesUsed - 1) << ";13H" << GREEN;
            }
        }

        ss << currentLine << BGTRUEBLK;
        return linesUsed;
    }

    int draw_wrapped_currentPresetName(std::stringstream& ss, const std::string& text, int termWidth, int startRow) {
        if (text.empty() || text == "None") return 0;

        std::stringstream words(text);
        std::string word;
        std::string currentLine = "";
        int linesUsed = 1;

        int maxLineWidth = termWidth - 13 - 2; 

        ss << "\033[" << startRow << ";10H" << BLUE << " * Milkdrop: " << GREEN;

        bool firstLine = true;
        while (words >> word) {
            int availableSpace = firstLine ? (maxLineWidth - 13) : maxLineWidth;

            if (currentLine.length() + word.length() + 1 <= (size_t)availableSpace) {
                if (!currentLine.empty()) currentLine += " ";
                currentLine += word;
            } else {
                ss << currentLine;
                linesUsed++;
                currentLine = word;
                firstLine = false;

                ss << "\033[" << (startRow + linesUsed - 1) << ";13H" << GREEN;
            }
        }

        ss << currentLine << BGTRUEBLK;
        return linesUsed;
    }
/*
    #ifdef USE_PROJECTM
    void update_visuals_logic() {
        if (!visualsRunning || !pm) return;
        if (!cfg.autoShuffleVisuals) return;

        uint32_t currentTime = SDL_GetTicks();

        // Check if 30 seconds have passed
        
        if (currentTime - lastPresetChange >= PRESET_DURATION) {
            load_random_preset(pm);
            lastPresetChange = currentTime;
           	needsRedraw = true;
        }
    }
    #endif
*/
    void draw_ui() {
        struct winsize w; ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        std::stringstream buffer;

        int mute;
        mpv_get_property(mpv, "mute", MPV_FORMAT_FLAG, &mute);
        std::string ismuteColor = mute ? "\033[91m" : "\033[92m";

        buffer << get_ui_header(w.ws_row);

        int currentRow = w.ws_row - 13;

        if (std::time(nullptr) < statusExpiry) {
            buffer << "\033[" << currentRow <<";10H" << GREEN << ">> " << statusMsg << "\n" << BLUE ;
            currentRow++;
        }

        int currentSongHeight = draw_wrapped_currentSong(buffer, currentSong, w.ws_col, currentRow);
        if (currentSongHeight > 0) {
            currentRow += currentSongHeight;
        }

        int descHeight = draw_wrapped_description(buffer, currentDesc, w.ws_col, currentRow);
        if (descHeight > 0) {
            currentRow += descHeight;
        }

        buffer << "\033[" << currentRow << ";10H" <<  BLUE << " * Station: " << niceGreenColor << currentStation;
        if (is_favorite()) buffer << BLUE << " " << "[\033[31mF\033[33ma\033[32mv\033[36mo\033[34m\033[35mr\033[31mi\033[33mt\033[32me\033[94m]" << BLUE;
        currentRow++;
        buffer << "\n" << "\033[" << currentRow << ";10H" <<  BLUE  << " * Listeners: " << niceGreenColor << currentListeners << "\n";
        currentRow++;
        buffer << "\033[" << currentRow << ";10H" <<  BLUE  << " * Total Channels: " << niceGreenColor << (int)channels.size() << "\n";
        currentRow++;
        buffer << "\033[" << currentRow << ";10H" <<  BLUE  << " * Favorites: " << niceGreenColor << count_favorites() << "\n";
        currentRow++;
        buffer << "\033[" << currentRow << ";10H" <<  BLUE  << " * Bitrate: " << niceGreenColor << get_bitrate_text() << "\n";
        currentRow++;
        buffer << "\033[" << currentRow << ";10H" <<  BLUE  << " * Vol: " << niceGreenColor << ismuteColor << get_vol_bar() << "\n";
        currentRow++;
        #ifdef USE_PROJECTM
        if (cfg.showVisuals) {
        int currentPresetNameHeight = draw_wrapped_currentPresetName(buffer, currentPresetName, w.ws_col, currentRow);
        if (currentPresetNameHeight > 0) {
            currentRow += currentPresetNameHeight; }
        }
        #endif
        buffer << get_ui_footer(w.ws_row);

        buffer << RESET;

        std::cout << buffer.str() << std::flush;
    }


    void save_favorite() {
        std::string home = getenv("HOME") ? getenv("HOME") : ".";
        #ifdef __HAIKU__
        std::string dir = home + "/config/settings/SuperMusicThingy";
        std::string path = dir + "/favorites.txt";
        #else
        std::string dir = home + "/.config/SuperMusicThingy";
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
        std::string path = home + "/config/settings/SuperMusicThingy/favorites.txt";
        #else
        std::string path = home + "/.config/SuperMusicThingy/favorites.txt";
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
        double original_vol;
        mpv_get_property(mpv, "volume", MPV_FORMAT_DOUBLE, &original_vol);
        fade_volume(mpv, 0, 300);

        currentSong = "Loading Favorite...";
        const char *cmd[] = {"loadfile", url.c_str(), NULL};
        mpv_command(mpv, cmd);
        fade_volume(mpv, original_vol, 500);
    }


    void delete_favorite() {
        std::string home = getenv("HOME") ? getenv("HOME") : ".";
        #ifdef __HAIKU__
        std::string path = home + "/config/settings/SuperMusicThingy/favorites.txt";
        #else
        std::string path = home + "/.config/SuperMusicThingy/favorites.txt";
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
        	#ifdef USE_PROJECTM
        	cmd = "notify --icon \"/boot/system/data/SuperMusicThingyNebula/icon/icon_24px.png\" --title \"SuperMusicThingyNebula\" \"" + station + ": " + song + "\" &";
        	#endif
        	#ifndef USE_PROJECTM
        	cmd = "notify --icon \"/boot/system/data/SuperMusicThingy/icon/icon_24px.png\" --title \"SuperMusicThingy\" \"" + station + ": " + song + "\" &";
        	#endif
        	
        #else
            #ifdef __linux__
                #ifdef USE_LIBNOTIFY
                    if (!notify_is_initted()) {
                        notify_init("SuperMusicThingy");
                    }

                    std::string body = station + "\n" + song;
                    GError* error = nullptr;
                    GdkPixbufLoader* loader = gdk_pixbuf_loader_new();
                    gdk_pixbuf_loader_write(loader, icon_24px_png, sizeof(icon_24px_png), nullptr);
                    gdk_pixbuf_loader_close(loader, nullptr);
                    GdkPixbuf* pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
                    NotifyNotification* notification = notify_notification_new(
                    "SuperMusicThingy",
                        body.c_str(),
                        nullptr
                        );
                    if (pixbuf) {
                        notify_notification_set_image_from_pixbuf(notification, pixbuf);
                    }
                    notify_notification_show(notification, &error);
                    g_object_unref(G_OBJECT(notification));
                    g_object_unref(G_OBJECT(loader));
                #endif
            #endif
       // cmd = "notify-send \"SuperMusicThingy\" \"" + station + "\n" + song + "\" &";
        #endif
        system(cmd.c_str());
    }

/*
   
    #if defined(__linux__)
    #include <thread>
    void listen_for_focus_requests() {
        while (true) {
            int client_fd = accept(global_socket_fd, nullptr, nullptr);
            if (client_fd >= 0) {
                char buffer[16] = {0};
                read(client_fd, buffer, sizeof(buffer) - 1);
              //  if (std::string(buffer) == "FOCUS") {
              //      system("wmctrl -a 'SuperMusicThingy'");

              //  }
                close(client_fd);
            }
        }
    }
    #endif

*/



    // --- Main Engine ---

    int main(int argc, char* argv[]) {
        // 1. FUNDAMENTAL LOAD (Always first)
        	load_config();
        	srand(time(0));

       // 2. Signal handler structure
   		   struct sigaction sa;
		   sa.sa_handler = handle_exit_signal;
 		   sigemptyset(&sa.sa_mask);
 		   sa.sa_flags = 0;

       // Register signals that trigger on window close or Ctrl+C
 		   sigaction(SIGHUP, &sa, NULL);  // Triggered when Terminal window is closed
 		   sigaction(SIGINT, &sa, NULL);  // Triggered by Ctrl+C
   		   sigaction(SIGTERM, &sa, NULL); // General termination request	


        // 3. CLI SENDER LOGIC 
        if (argc > 1) {
            std::string cmd = argv[1]; 
            int fd = open(fifoPath, O_WRONLY | O_NONBLOCK);

            if (fd == -1) {
                std::cerr << "SuperMusicThingy is not running." << std::endl;
                return 1;
            }
            if (cmd == "help" || cmd == "--help" || cmd == "-h") {
                std::cout << BLUE << "\n--- SuperMusicThingy CLI Help ---" << BLUE  << "\n"
                << "--------------------------\n" << BLUE
                << "Usage: SuperMusicThingy ["  << niceGreenColor << "command" << BLUE << "]\n\n" << BLUE
                << "Commands:\n"
                << niceGreenColor << "  status        " << BLUE << "  - Show current song, volume, and visualizer preset\n" << BLUE
                << niceGreenColor << "  shuffle       " << BLUE << "  - Skip to the next song in the queue\n" << BLUE
                #ifdef USE_PROJECTM  
                << niceGreenColor << "  visual        " << BLUE << "  - Shuffle to a new random Milkdrop preset\n" << BLUE
                #endif
                << niceGreenColor << "  vol_up        " << BLUE << "  - Increase volume\n" << BLUE
                << niceGreenColor << "  vol_down      " << BLUE << "  - Decrease volume\n" << BLUE
                << niceGreenColor << "  mute          " << BLUE << "  - Toggle audio\n" << BLUE
                << niceGreenColor << "  toggle        " << BLUE << "  - Play/Pause the music\n" << BLUE
                << niceGreenColor << "  stop          " << BLUE << "  - Stop the music\n" << BLUE
                << niceGreenColor << "  quit          " << BLUE << "  - Close the running SuperMusicThingy instance\n" << BLUE
                << "--------------------------\n" << std::endl;
                return 0; 
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
                    usleep(33333);
                }
                close(respFd); unlink(respPath);
                return 1;
            }

            // For all other commands (shuffle, quit, etc.)
            write(fd, cmd.c_str(), cmd.length());
            close(fd);
            return 0; 
        }

        // 4. TERMINAL WRAPPER (Ensures we are in a visible window)
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
                {"konsole", "--title \"SuperMusicThingy\" -e"},
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
            // Title set
            std::cout << "\033]0;SuperMusicThingy\007" << std::flush;

        // Linux check
       #if defined(__linux__)
       if (is_already_running_linux()) {
       std::cout << "Another instance is already running. " << std::endl;
       return 0; 
        }
        #endif


        // 5. ACTUAL PLAYER INITIALIZATION
        init_mpv();        
        fetch_channels(); 


        #ifdef USE_PROJECTM
        if (cfg.showVisuals and !is_native_tty()) {
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



        // 7. THE MAIN LOOP
        while (true) {
            bool needsRedraw = false;

          // A. --- VISUALS LOGIC ---
            #ifdef USE_PROJECTM
            
            if (visualsRunning && pm) {
                // Random preset every 30s
       			if (cfg.autoShuffleVisuals) { 
       				 uint32_t currentTime = SDL_GetTicks();
        				// Check if 30 seconds have passed
        			if (currentTime - lastPresetChange >= PRESET_DURATION) {
           				load_random_preset(pm);
            			lastPresetChange = currentTime;
           				needsRedraw = true;
        			}
      			  }            


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
                // Handle window events
                SDL_Event e;
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) {
                        visualsRunning = false;
                    }
                    // --- NEW: for resizing ---
                    else if (e.type == SDL_WINDOWEVENT) {
                        if (e.type == SDL_QUIT || (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)) {

                            visualsRunning = false; // Stop the logic

                            // --- KILL THE WINDOW NOW ---
                            if (glContext) {
                                SDL_GL_DeleteContext(glContext);
                                glContext = nullptr;
                            }
                            if (visualWin) {
                                SDL_DestroyWindow(visualWin);
                                visualWin = nullptr;
                            }

                            needsRedraw = true;
                        }

                        if (e.window.event == SDL_WINDOWEVENT_RESIZED ||
                            e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {

                        int newW = e.window.data1;
                        int newH = e.window.data2;

                        // 1. Tell OpenGL the new drawing area
                        glViewport(0, 0, newW, newH);

                        // 2. Tell projectM the new internal resolution
                        projectm_set_window_size(pm, newW, newH);

                            }
                    }
                    // FULL SCREEN KEY EVENTS
                    // Shuffle visual effeets
                    else if (e.type == SDL_KEYDOWN) {

                        if (e.key.keysym.sym == SDLK_v) {
                            load_random_preset(pm);
                            lastPresetChange = SDL_GetTicks();
                            needsRedraw = true;
                        }

                        // Shuffle
                        else if (e.key.keysym.sym == SDLK_s) {
                            play_random();
                            currentSong = "Buffering...";
                            needsRedraw = true;
                        }
                        // Play favorite
                        else if (e.key.keysym.sym == SDLK_f) {
                            play_favorite();
                            needsRedraw = true;
                        }

                        // Play mute
                        else if (e.key.keysym.sym == SDLK_m) {
                            const char* cmd_mute[] = {"cycle", "mute", NULL};
                            mpv_command(mpv, cmd_mute);
                            needsRedraw = true;
                        }

                        // Stop
                        else if (e.key.keysym.sym == SDLK_x) {
                            const char* cmd_stop[] = {"stop", NULL};
                            mpv_command(mpv, cmd_stop);
                            needsRedraw = true;
                        }

                        // Toggle/pause
                        else if (e.key.keysym.sym == SDLK_p) {
                            const char* cmd_pause[] = {"cycle", "pause", NULL};
                            mpv_command(mpv, cmd_pause);
                            needsRedraw = true;
                        }
                        // Vol up
                        else if (e.key.keysym.sym == SDLK_EQUALS || e.key.keysym.sym == SDLK_KP_PLUS) {
                            set_volume('+');
                            needsRedraw = true;
                        }
                        // Vol down
                        else if (e.key.keysym.sym == SDLK_MINUS || e.key.keysym.sym == SDLK_KP_MINUS) {
                            set_volume('-');
                            needsRedraw = true;
                        }

                        // K key exists fullscreen
                        else if (e.key.keysym.sym == SDLK_k) {

                            uint32_t flags = SDL_GetWindowFlags(visualWin);
                            bool isFullscreen = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
                            SDL_SetWindowFullscreen(visualWin, isFullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                            SDL_ShowCursor(isFullscreen ? SDL_ENABLE : SDL_DISABLE);
                            int w, h;
                            SDL_GetWindowSize(visualWin, &w, &h);
                            glViewport(0, 0, w, h);
                            projectm_set_window_size(pm, w, h);
                            needsRedraw = true;
                        }

                        // Esc key
                        else if (e.key.keysym.sym == SDLK_ESCAPE) {
                            uint32_t flags = SDL_GetWindowFlags(visualWin);
                            bool isFullscreen = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP);

                            // 2. Toggle it off
                            if (isFullscreen) {
                                SDL_SetWindowFullscreen(visualWin, 0);
                                SDL_ShowCursor(SDL_ENABLE);
                                // 3. Re-sync dimensions
                                int w, h;
                                SDL_GetWindowSize(visualWin, &w, &h);
                                glViewport(0, 0, w, h);
                                projectm_set_window_size(pm, w, h);
                                needsRedraw = true;
                            }
                        }
                    }

                    //
                    else if (e.type == SDL_MOUSEBUTTONDOWN) {
                        // Check if it's the Left Mouse Button and a Double Click (2)
                        if (e.button.button == SDL_BUTTON_LEFT && e.button.clicks == 2) {

                            // 1. Get current window flags
                            uint32_t flags = SDL_GetWindowFlags(visualWin);
                            bool isFullscreen = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
                            SDL_SetWindowFullscreen(visualWin, isFullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                            SDL_ShowCursor(isFullscreen ? SDL_ENABLE : SDL_DISABLE);

                            // 3. Immediately update projectM/OpenGL with new dimensions
                            int w, h;
                            SDL_GetWindowSize(visualWin, &w, &h);
                            glViewport(0, 0, w, h);
                            projectm_set_window_size(pm, w, h);
                            needsRedraw = true;
                        }
                    }
                }

                if (needsRedraw) {                    
                    draw_ui();       
                    needsRedraw = false; 
                }
            }
            #endif




            // B. Notifications
            if (notifyTimer > 0 && std::time(nullptr) >= notifyTimer) {
                currentSong = pendingSong;
               
                if (cfg.showNotifications) {
                    send_notification(currentStation, currentSong);
                }

                notifyTimer = 0; // Reset the fuse
                needsRedraw = true;
            }


            // C. FIFO LISTENER
            char cmdBuf[64]; // Buffer for incoming commands
            ssize_t bytes = read(fifoFd, cmdBuf, sizeof(cmdBuf) - 1);
            if (bytes > 0) {
                cmdBuf[bytes] = '\0';
                std::string cmd(cmdBuf);
                if (cmd == "status") {
                    int respFd = open(respPath, O_WRONLY | O_NONBLOCK);
                    if (respFd != -1) {
                        std::stringstream ss;
                        ss << BLUE << "\n--- SuperMusicThingy Status ---" << BLUE  << "\n"
                        << BLUE << "Song:      " << niceGreenColor << currentSong << RESET << "\n"
                        << BLUE << "Desc:      " << niceGreenColor << currentDesc << RESET << "\n"
                        << BLUE << "Station:   " << niceGreenColor << currentStation << RESET << "\n"
                        << BLUE << "Listeners: " << niceGreenColor << currentListeners << RESET << "\n"
                        << BLUE << "Total Ch:  " << niceGreenColor << channels.size() << RESET << "\n"
                        << BLUE << "Favorites: " << niceGreenColor << count_favorites() << RESET << "\n"
                        << BLUE << "Quality:   " << niceGreenColor << get_bitrate_text() << RESET << "\n"
                        << BLUE << "Volume:    " << niceGreenColor << get_vol_bar() << RESET << "\n";
                        #ifdef USE_PROJECTM
                        if (visualsRunning) {
                            ss << std::string(BLUE) << "Visual:    " << std::string(niceGreenColor)
                            << currentPresetName << std::string(RESET) << "\n";
                        }
                        #endif

                        ss << BLUE << "---------------------------" << RESET ;

                        std::string reply = ss.str();
                        write(respFd, reply.c_str(), reply.length());
                        close(respFd);
                    }
                }

                else if (cmd == "toggle") {
                    const char* cmd_pause[] = {"cycle", "pause", NULL};
                    mpv_command(mpv, cmd_pause);
                    needsRedraw = true;
                }
                else if (cmd == "stop") {
                    const char* cmd_stop[] = {"stop", NULL};
                    mpv_command(mpv, cmd_stop);
                }
                else if (cmd == "favorites") {
                    play_favorite(); 
                    needsRedraw = true;
                }
                else if (cmd == "add_fav") {
                    save_favorite();
                }
                else if (cmd == "visual") {
                	    #ifdef USE_PROJECTM    
                        if (visualsRunning && !is_native_tty()) { 
                        load_random_preset(pm);
                        lastPresetChange = SDL_GetTicks();
                        needsRedraw = true;
                        }
                        #endif
                }
                else if (cmd == "del_fav") {
                    delete_favorite();
                }
                else if (cmd == "quit") {
                    goto end;                 }
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
                else if (cmd == "mute") {
                    const char* cmd_mute[] = {"cycle", "mute", NULL};
                    mpv_command(mpv, cmd_mute);
                    needsRedraw = true;
                }

            }


            // D. MENU SCREENS
            if (showConfig) {
                showConfig = draw_config_menu();
                if (!showConfig) {
                    draw_ui();
                }
                if (is_native_tty()) {
                    usleep(500000); // 2FPS // Have to slow things down a bit
                } else {
                    usleep(33333);  // 30 FPS
                }
                continue;
            }

            if (showHelp) {
                showHelp = draw_help_menu();
                if (!showHelp) {
                    draw_ui();
                }

                if (is_native_tty()) {
                    usleep(500000); // 2FPS
                } else {
                    usleep(33333);  // 30 FPS
                }
                continue;
            }

            if (showFavorites) {
                    showFavorites = draw_favorites_menu();
                if (!showFavorites) {
                    draw_ui();
                }

                if (is_native_tty()) {
                    usleep(500000); // 2FPS
                } else {
                    usleep(33333);  // 30 FPS
                }
                continue;
            }


            // E. STATUS EXPIRY
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


            // F. MPV EVENTS (Buffered + Delayed Notification Version)
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



            // G. KEYBOARD INPUT
            if (kbhit()) {
                char input = getchar(); // Get the raw key once
                char c = std::tolower(input); // Create a lowercase version for the switch

                if (c == 'q') break; 
                switch (c) {
                    case 'l': showFavorites = true; selectedFav = 0; currentMenu = FAVORITES; break;
                    case 's': play_random(); break;
                    case 'a': save_favorite(); break;
                    case 'c': showConfig = true; currentMenu = CONFIG;  break;
                    case 'f': play_favorite(); break;
                    case 'd': delete_favorite(); break;
                    case 'x': {
                        const char* cmd_stop[] = {"stop", NULL};
                        mpv_command(mpv, cmd_stop);
                        currentSong = "Stopped"; 
                        break;
                    }
                    case 'p': {
                        const char* cmd_pause[] = {"cycle", "pause", NULL};
                        mpv_command(mpv, cmd_pause);
                        break;
                    }

                   	#ifdef USE_PROJECTM
                    case 'v':
                   			if (cfg.showVisuals) {
                            if (!visualsRunning && !is_native_tty()) { init_visuals(); }
                            }

                        load_random_preset(pm);
                        lastPresetChange = SDL_GetTicks();
                        break;

                    #endif
                    case 'h': showHelp = true; currentMenu = HELP; break;
                    case 'n': play_random(); break;
                    case '+': case '-':
                        set_volume(input); // Use 'input' here so '+' works correctly
                        break;
                    case 'm': toggle_mute(); break;

                    #ifdef USE_PROJECTM
                    case 'k': {
                        // Don't crash if visual screen not open
                        if (!visualsRunning and !is_native_tty()) break;

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

                usleep(33333);  // 30 FPS

        } //End The Main Loop


        end:
        system("stty cooked echo");

        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        std::stringstream buffer;
        buffer << "\033[48;2;0;0;0m" << BLUE << "\033[2J\033[3J\033[H";
        buffer << get_ui_footer(w.ws_row) << BLUE << "Good bye! " << RESET << std::endl;
        std::cout << buffer.str();

        if (mpv) mpv_terminate_destroy(mpv);
        return 0;
    }
