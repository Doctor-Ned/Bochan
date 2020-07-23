#pragma once

#include "AudioCoder.h"

namespace bochan {
    class BOCHANAPI AudioEncoder : public AudioCoder {
    public:
        AudioEncoder(BufferPool* bufferPool);
        virtual bool initialize(BochanCodec bochanCodec, int sampleRate, unsigned long long bitRate) = 0;
        virtual int getSamplesPerFrame() const = 0;
        virtual int getInputBufferByteSize() const = 0;
        virtual bool hasExtradata() = 0;
        virtual ByteBuffer* getExtradata() = 0;
        virtual std::vector<ByteBuffer*> encode(ByteBuffer* samples) = 0;
    };
}