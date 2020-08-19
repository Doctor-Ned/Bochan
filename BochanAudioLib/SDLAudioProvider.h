#pragma once

#include "AudioProvider.h"

#include "SDL.h"

namespace bochan {
    class SDLAudioProvider sealed : public AudioProvider {
    public:
        BOCHANAPI SDLAudioProvider();
        BOCHANAPI ~SDLAudioProvider();
        BOCHANAPI bool initialize(gsl::cstring_span audioDevice, int sampleRate, size_t bufferSize, bool forceMono);
        BOCHANAPI void deinitialize() override;
        BOCHANAPI bool isInitialized() const override;
        BOCHANAPI bool isRecording() const;
        BOCHANAPI bool record();
        BOCHANAPI void stop();
        BOCHANAPI void flush();
        BOCHANAPI bool fillBuffer(gsl::not_null<ByteBuffer*> buff) override;
        BOCHANAPI std::vector<gsl::cstring_span> getAvailableDevices() const;
    private:
        BOCHANAPI void reduceBuffer(size_t size);
        BOCHANAPI static void audioCallback(void* ptr, Uint8* stream, int len);
        int sampleRate{ 0 };
        bool initialized{ false }, recording{ false }, forceMono{ false };
        gsl::cstring_span audioDevice{ nullptr };
        gsl::span<uint8_t> sampleBuffer{};
        std::atomic<size_t> sampleBufferPos{ 0ULL };
        SDL_AudioDeviceID devId{ 0U };
        std::recursive_mutex bufferMutex{};
    };
}
