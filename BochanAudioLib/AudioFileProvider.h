#pragma once

#include "AudioProvider.h"

#include <chrono>

extern "C" {
#include <libswresample\swresample.h>
#include <libavformat\avformat.h>
}

namespace bochan {
    class AudioFileProvider sealed : public AudioProvider {
    public:
        AudioFileProvider() = default;
        BOCHANAPI ~AudioFileProvider();
        BOCHANAPI bool initialize(const char* fileName, int sampleRate, size_t bufferSize);
        BOCHANAPI void deinitialize() override;
        BOCHANAPI bool isInitialized() const override;
        BOCHANAPI bool fillBuffer(ByteBuffer* buff) override;
        BOCHANAPI bool isSimulatingTime() const;
        BOCHANAPI void setSimulateTime(bool simulateTime);
    private:
        BOCHANAPI bool readFrame();
        BOCHANAPI void reduceBuffer(size_t size);
        bool initialized{ false };
        const char* fileName{ nullptr };
        int sampleRate{ 0 }, streamId{ -1 };
        bool startPointAvailable{ false }, simulateTime{ true };
        std::chrono::system_clock::time_point startPoint{};
        size_t bufferSize{ 0ULL }, bufferPos{ 0ULL };
        uint8_t* internalBuffer{ nullptr };
        std::recursive_mutex bufferMutex{};
        AVPacket* packet{ nullptr };
        AVFrame* frame{ nullptr }, * resampledFrame{ nullptr };
        AVFormatContext* formatContext{ nullptr };
        AVCodec* codec{ nullptr };
        AVCodecContext* context{ nullptr };
        SwrContext* swrContext{ nullptr };
    };
}
