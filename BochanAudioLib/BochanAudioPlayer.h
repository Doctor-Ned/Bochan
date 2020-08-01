#pragma once

#include "AudioPlayer.h"
#include "SDL.h"

namespace bochan {
    class BOCHANAPI BochanAudioPlayer sealed : public AudioPlayer{
    public:
        BochanAudioPlayer() = default;
        ~BochanAudioPlayer();
        bool init(int sampleRate, size_t minBufferSize, size_t maxBufferSize) override;
        void deinitialize() override;
        bool isInitialized() const override;
        size_t queueData(ByteBuffer* buff) override;
        bool isPlaying() override;
        void play() override;
        void stop() override;
        void flush() override;
    private:
        static void fillData(void* ptr, Uint8* stream, int len);
        bool initialized{ false }, playing{ false };
        uint8_t* sampleBuffer{ nullptr };
        size_t sampleBufferPos{ 0ULL };
        std::mutex bufferMutex{};
    };
}
