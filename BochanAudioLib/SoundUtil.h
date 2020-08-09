#pragma once

#include "pch.h"
#include "SDL.h"

namespace bochan {
    class SoundUtil sealed {
    public:
        SoundUtil() = delete;
        BOCHANAPI static bool initAudio(void* owner);
        BOCHANAPI static void quitAudio(void* owner);
        BOCHANAPI static std::vector<const char*> getAudioDrivers();
        static const char* AUDIO_DRIVER;
    private:
        static std::vector<void*> audioInvokers;
    };
}