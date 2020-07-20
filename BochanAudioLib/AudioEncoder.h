#pragma once

#include "BochanCodec.h"

namespace bochan {
    class BOCHANAPI AudioEncoder {
    public:
        AudioEncoder() = default;
        virtual ~AudioEncoder() = default;
        virtual bool initialize(BochanCodec bochanCodec, int sampleRate, unsigned long long bitRate) = 0;
        virtual void deinitialize() = 0;
        virtual bool isInitialized() const = 0;
        virtual BochanCodec getCodec() const = 0;
        virtual int getSampleRate() const = 0;
        virtual unsigned long long getBitRate() const = 0;
        virtual int getSamplesPerFrame() const = 0;
        virtual std::vector<ByteBuffer*> encode(Buffer<uint16_t>* samples) = 0;
    };
}