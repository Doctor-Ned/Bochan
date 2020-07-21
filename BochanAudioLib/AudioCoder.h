#pragma once

#include "BochanCodec.h"
#include "BufferPool.h"

namespace bochan {
    class BOCHANAPI AudioCoder {
    public:
        AudioCoder(BufferPool* bufferPool);
        virtual ~AudioCoder() = default;
        virtual bool initialize(BochanCodec bochanCodec, int sampleRate, unsigned long long bitRate) = 0;
        virtual void deinitialize() = 0;
        virtual bool isInitialized() const = 0;
        virtual BochanCodec getCodec() const = 0;
        virtual int getSampleRate() const = 0;
        virtual unsigned long long getBitRate() const = 0;
    protected:
        ByteBuffer* samplesToFloat(ByteBuffer* samples);
        ByteBuffer* floatToSamples(ByteBuffer* fltp);
        void samplesToFloat(ByteBuffer* from, float* to);
        void floatToSamples(ByteBuffer* from, int16_t* to);
        BufferPool* bufferPool{ nullptr };
    };
}