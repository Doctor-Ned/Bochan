#pragma once

#include "AudioEncoder.h"

extern "C" {
#include <libavformat\avformat.h>
}

namespace bochan {
    class BOCHANAPI BochanEncoder : public AudioEncoder {
    public:
        BochanEncoder();
        ~BochanEncoder();
        bool initialize(BochanCodec bochanCodec, int sampleRate, unsigned long long bitRate) override;
        void deinitialize() override;
        bool isInitialized() override;
        BochanCodec getCodec() override;
        int getSampleRate() override;
        unsigned long long getBitRate() override;
    private:
        bool initialized{ false };
        BochanCodec bochanCodec{ BochanCodec::None };
        int sampleRate{ 0 };
        unsigned long long bitRate{ 0ULL };
        AVCodecID codecId{};
        AVCodec* codec{ nullptr };
        AVCodecContext* context{ nullptr };
        AVPacket* packet{ nullptr };
        AVFrame* frame{ nullptr };
    };
}