#pragma once

#include "AudioCoder.h"

namespace bochan {
    class BOCHANAPI AudioDecoder : public AudioCoder {
    public:
        AudioDecoder(BufferPool& bufferPool);
        virtual bool initialize(const CodecConfig& config, bool saveToFile, ByteBuffer* extradata) = 0;
        virtual bool needsExtradata(BochanCodec bochanCodec) = 0;
        virtual std::vector<ByteBuffer*> decode(ByteBuffer* samples) = 0;
    };
}