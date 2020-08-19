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
        BOCHANAPI bool initialize(gsl::cstring_span fileName, int sampleRate, size_t bufferSize);
        BOCHANAPI void deinitialize() override;
        BOCHANAPI bool isInitialized() const override;
        BOCHANAPI bool fillBuffer(gsl::not_null<ByteBuffer*> buff) override;
        BOCHANAPI bool isSimulatingTime() const;
        BOCHANAPI void setSimulateTime(bool simulateTime);
        BOCHANAPI double getDuration();
        BOCHANAPI double getPositionSeconds();
        BOCHANAPI bool setPositionSeconds(double position);
        BOCHANAPI bool rewindForward(double seconds);
        BOCHANAPI bool rewindBackward(double seconds);
        BOCHANAPI bool rewindToStart();
        BOCHANAPI bool isEof();
    private:
        BOCHANAPI bool seekPos(int64_t minPos, int64_t pos, int64_t maxPos);
        BOCHANAPI bool readFrame();
        BOCHANAPI void reduceBuffer(size_t size);
        bool initialized{ false };
        gsl::cstring_span fileName{ nullptr };
        int sampleRate{ 0 }, streamId{ -1 };
        bool startPointAvailable{ false }, simulateTime{ true };
        std::atomic_bool eof{ false };
        std::chrono::system_clock::time_point startPoint{};
        gsl::span<uint8_t> internalBuffer{ };
        size_t bufferPos{ 0ULL };
        int bytesPerSample{ 0 };
        std::recursive_mutex formatMutex{};
        AVPacket* packet{ nullptr };
        AVFrame* frame{ nullptr }, * resampledFrame{ nullptr };
        AVFormatContext* formatContext{ nullptr };
        AVCodec* codec{ nullptr };
        AVCodecContext* context{ nullptr };
        SwrContext* swrContext{ nullptr };
    };
}
