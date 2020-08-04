#pragma once

#include "AudioDecoder.h"
#include "BufferPool.h"

extern "C" {
#include <libavformat\avformat.h>
}

namespace bochan {
    class BOCHANAPI BochanDecoder sealed : public AudioDecoder{
    public:
        BochanDecoder(BufferPool& bufferPool);
        ~BochanDecoder();
        bool initialize(const CodecConfig& config, ByteBuffer* extradata) override;
        void deinitialize() override;
        bool isInitialized() const override;
        bool needsExtradata(BochanCodec bochanCodec) override;
        std::vector<ByteBuffer*> decode(ByteBuffer* samples) override;
    private:
        bool initialized{ false };
        int bytesPerSample{ 0 };
        AVCodecConfig avCodecConfig{};
        AVCodec* codec{ nullptr };
        AVCodecContext* context{ nullptr };
        AVCodecParserContext* parser{ nullptr };
        AVPacket* packet{ nullptr };
        AVFrame* frame{ nullptr };
    };
}

