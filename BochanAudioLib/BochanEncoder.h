#pragma once

#include "AudioEncoder.h"
#include "BufferPool.h"

extern "C" {
#include <libavformat\avformat.h>
}

namespace bochan {
    class BOCHANAPI BochanEncoder : public AudioEncoder {
    public:
        BochanEncoder(BufferPool* bufferPool);
        ~BochanEncoder();
        bool initialize(BochanCodec bochanCodec, int sampleRate, unsigned long long bitRate) override;
        void deinitialize() override;
        bool isInitialized() const override;
        BochanCodec getCodec() const override;
        int getSampleRate() const override;
        unsigned long long getBitRate() const override;
        int getSamplesPerFrame() const override;
        std::vector<ByteBuffer*> encode(Buffer<uint16_t>* samples) override;
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
        BufferPool* bufferPool{ nullptr };
    };
}