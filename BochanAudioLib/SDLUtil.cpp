#include "pch.h"
#include "SDLUtil.h"

const char* bochan::SDLUtil::AUDIO_DRIVER{ "directsound" };
std::vector<void*> bochan::SDLUtil::audioInvokers{};

bool bochan::SDLUtil::initAudio(void* owner) {
    if (audioInvokers.empty()) {
        BOCHAN_DEBUG("Initializing SDL audio with driver '{}'...", AUDIO_DRIVER);
        if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            BOCHAN_CRITICAL("Failed to init audio subsystem: {}", SDL_GetError());
            return false;
        }
        if (SDL_AudioInit(AUDIO_DRIVER)) {
            BOCHAN_CRITICAL("Failed to init audio driver: {}", SDL_GetError());
            return false;
        }
        audioInvokers.push_back(owner);
    } else if (std::find(audioInvokers.begin(), audioInvokers.end(), owner) == audioInvokers.end()) {
        audioInvokers.push_back(owner);
    }
    return true;
}

void bochan::SDLUtil::quitAudio(void* owner) {
    std::vector<void*>::iterator it{ std::find(audioInvokers.begin(), audioInvokers.end(), owner) };
    if (it != audioInvokers.end()) {
        audioInvokers.erase(it);
    }
    if (audioInvokers.empty()) {
        BOCHAN_DEBUG("Quitting SDL audio...");
        SDL_AudioQuit();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
}

std::vector<const char*> bochan::SDLUtil::getAudioDrivers() {
    std::vector<const char*> result{};
    const int DRIVERS = SDL_GetNumAudioDrivers();
    for (int i = 0; i < DRIVERS; ++i) {
        result.push_back(SDL_GetAudioDriver(i));
    }
    return result;
}
