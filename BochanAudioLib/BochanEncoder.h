#pragma once

#include "AudioEncoder.h"

extern "C" {
#include <libavformat\avformat.h>
}

namespace bochan {
    class BOCHANAPI BochanEncoder sealed : public AudioEncoder{
    public:
        BochanEncoder(BufferPool * bufferPool);
        ~BochanEncoder();
        bool initialize(BochanCodec bochanCodec, int sampleRate, unsigned long long bitRate) override;
        void deinitialize() override;
        bool isInitialized() const override;
        BochanCodec getCodec() const override;
        int getSampleRate() const override;
        unsigned long long getBitRate() const override;
        int getSamplesPerFrame() const override;
        int getInputBufferByteSize() const override;
        bool hasExtradata() override;
        ByteBuffer* getExtradata() override;
        std::vector<ByteBuffer*> encode(ByteBuffer* samples) override;
    private:
        bool initialized{ false };
        int bytesPerSample{0};
        AVSampleFormat sampleFormat{ AVSampleFormat::AV_SAMPLE_FMT_NONE };
        AVCodecID codecId{};
        AVCodec* codec{ nullptr };
        AVCodecContext* context{ nullptr };
        AVPacket* packet{ nullptr };
        AVFrame* frame{ nullptr };
    };
}