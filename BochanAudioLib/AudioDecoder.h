#pragma once

#include "AudioCoder.h"

namespace bochan {
    class BOCHANAPI AudioDecoder : public AudioCoder {
    public:
        AudioDecoder(BufferPool* bufferPool);
        virtual bool initialize(BochanCodec bochanCodec, int sampleRate, unsigned long long bitRate, ByteBuffer* extradata) = 0;
        virtual bool needsExtradata(BochanCodec bochanCodec) = 0;
        virtual std::vector<ByteBuffer*> decode(ByteBuffer* samples) = 0;
    };
}