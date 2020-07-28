#pragma once

#include "AudioDecoder.h"
#include "BufferPool.h"

extern "C" {
#include <libavformat\avformat.h>
}

namespace bochan {
    class BOCHANAPI BochanDecoder sealed : public AudioDecoder{
    public:
        BochanDecoder(BufferPool * bufferPool);
        ~BochanDecoder();
        bool initialize(BochanCodec bochanCodec, int sampleRate, unsigned long long bitRate, ByteBuffer* extradata) override;
        void deinitialize() override;
        bool isInitialized() const override;
        BochanCodec getCodec() const override;
        int getSampleRate() const override;
        unsigned long long getBitRate() const override;
        bool needsExtradata(BochanCodec bochanCodec) override;
        std::vector<ByteBuffer*> decode(ByteBuffer* samples) override;
    private:
        bool initialized{ false };
        int bytesPerSample{ 0 };
        AVSampleFormat sampleFormat{ AVSampleFormat::AV_SAMPLE_FMT_NONE };
        AVCodecID codecId{};
        AVCodec* codec{ nullptr };
        AVCodecContext* context{ nullptr };
        AVCodecParserContext* parser{ nullptr };
        AVPacket* packet{ nullptr };
        AVFrame* frame{ nullptr };
    };
}

