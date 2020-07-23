#pragma once

#include "AudioCoder.h"

namespace bochan {
    class BOCHANAPI AudioEncoder : public AudioCoder {
    public:
        AudioEncoder(BufferPool* bufferPool);
        virtual int getSamplesPerFrame() const = 0;
        virtual int getInputBufferByteSize() const = 0;
        virtual bool hasExtradata() = 0;
        virtual ByteBuffer* getExtradata() = 0;
        virtual std::vector<ByteBuffer*> encode(ByteBuffer* samples) = 0;
    };
}