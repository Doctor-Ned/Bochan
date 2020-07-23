#pragma once

#include "AudioCoder.h"

namespace bochan {
    class BOCHANAPI AudioDecoder : public AudioCoder {
    public:
        AudioDecoder(BufferPool* bufferPool);
        virtual bool needsExtradata() = 0;
        virtual void setExtradata(ByteBuffer* extradata) = 0;
        virtual std::vector<ByteBuffer*> decode(ByteBuffer* samples) = 0;
    };
}