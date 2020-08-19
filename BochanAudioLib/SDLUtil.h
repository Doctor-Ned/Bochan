#pragma once

#include "pch.h"
#include "SDL.h"

namespace bochan {
    class SDLUtil sealed {
    public:
        SDLUtil() = delete;
        BOCHANAPI static bool initAudio(void* owner);
        BOCHANAPI static void quitAudio(void* owner);
        BOCHANAPI static std::vector<gsl::cstring_span> getAudioDrivers();
        static const char* AUDIO_DRIVER;
    private:
        static std::vector<void*> audioInvokers;
    };
}