#pragma once

#include "AudioPlayer.h"

#include "SDL.h"

namespace bochan {
    class SDLAudioPlayer sealed : public AudioPlayer {
    public:
        BOCHANAPI SDLAudioPlayer();
        BOCHANAPI ~SDLAudioPlayer();
        SDLAudioPlayer(SDLAudioPlayer&) = delete;
        SDLAudioPlayer(SDLAudioPlayer&&) = delete;
        SDLAudioPlayer& operator=(SDLAudioPlayer&) = delete;
        SDLAudioPlayer& operator=(SDLAudioPlayer&&) = delete;
        BOCHANAPI bool initialize(const char* audioDevice, int sampleRate, size_t minBufferSize, size_t maxBufferSize) override;
        BOCHANAPI void deinitialize() override;
        BOCHANAPI bool isInitialized() const override;
        BOCHANAPI size_t queueData(gsl::not_null<ByteBuffer*> buff) override;
        BOCHANAPI bool isPlaying() override;
        BOCHANAPI bool play() override;
        BOCHANAPI void stop() override;
        BOCHANAPI void flush() override;
        BOCHANAPI std::vector<const char*> getAvailableDevices() const override;
    private:
        BOCHANAPI static void fillData(void* ptr, Uint8* stream, int len);
        bool initialized{ false }, playing{ false };
        const char* audioDevice{ nullptr };
        uint8_t* sampleBuffer{ nullptr };
        size_t sampleBufferPos{ 0ULL };
        SDL_AudioDeviceID devId{ 0U };
        std::mutex bufferMutex{};
    };
}
