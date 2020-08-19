#pragma once

#include "AudioEncoder.h"

extern "C" {
#include <libavformat\avformat.h>
}

namespace bochan {
    class BOCHANAPI BochanEncoder sealed : public AudioEncoder{
    public:
        BochanEncoder(BufferPool & bufferPool);
        ~BochanEncoder();
        BochanEncoder(BochanEncoder&) = delete;
        BochanEncoder(BochanEncoder&&) = delete;
        BochanEncoder& operator=(BochanEncoder&) = delete;
        BochanEncoder& operator=(BochanEncoder&&) = delete;
        bool initialize(const CodecConfig& config) override;
        void deinitialize() override;
        bool isInitialized() const override;
        int getSamplesPerFrame() const override;
        int getInputBufferByteSize() const override;
        bool hasExtradata() override;
        ByteBuffer* getExtradata() override;
        std::vector<AudioPacket> encode(gsl::not_null<ByteBuffer*> samples) override;
    private:
        bool initialized{ false };
        int bytesPerSample{0};
        int64_t pts{ 0 };
        AVCodecConfig avCodecConfig{};
        AVCodec* codec{ nullptr };
        AVCodecContext* context{ nullptr };
        AVPacket* packet{ nullptr };
        AVFrame* frame{ nullptr };
    };
}