#pragma once

#include "AudioDecoder.h"
#include "BufferPool.h"

extern "C" {
#include <libavformat\avformat.h>
}

namespace bochan {
    class BOCHANAPI BochanDecoder sealed : public AudioDecoder{
    public:
        BochanDecoder(BufferPool & bufferPool);
        ~BochanDecoder();
        BochanDecoder(BochanDecoder&) = delete;
        BochanDecoder(BochanDecoder&&) = delete;
        BochanDecoder& operator=(BochanDecoder&) = delete;
        BochanDecoder& operator=(BochanDecoder&&) = delete;
        bool initialize(const CodecConfig& config, bool saveToFile, ByteBuffer* extradata) override;
        void deinitialize() override;
        bool isInitialized() const override;
        bool needsExtradata(BochanCodec bochanCodec) override;
        std::vector<ByteBuffer*> decode(ByteBuffer* samples) override;
    private:
        bool initialized{ false }, saveToFile{ false };
        int bytesPerSample{ 0 };
        int64_t pts{ 0 };
        int64_t dts{ 0 };
        AVCodecConfig avCodecConfig{};
        AVCodec* codec{ nullptr };
        AVCodecContext* context{ nullptr };
        AVCodecParserContext* parser{ nullptr };
        AVIOContext* avioContext{ nullptr };
        AVFormatContext* formatContext{ nullptr };
        AVStream* stream{ nullptr };
        AVPacket* packet{ nullptr };
        AVFrame* frame{ nullptr };
    };
}

