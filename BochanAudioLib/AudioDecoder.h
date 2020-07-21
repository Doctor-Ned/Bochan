#pragma once

#include "AudioCoder.h"

namespace bochan {
    class BOCHANAPI AudioDecoder : public AudioCoder {
    public:
        AudioDecoder(BufferPool* bufferPool);
        virtual std::vector<ByteBuffer*> decode(ByteBuffer* samples) = 0;
    };
}