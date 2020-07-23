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
        ByteBuffer* int16ToFloat(ByteBuffer* samples);
        ByteBuffer* floatToInt16(ByteBuffer* fltp);
        void int16ToFloat(ByteBuffer* from, float* to);
        void floatToInt16(ByteBuffer* from, int16_t* to);
        void int16ToFloat(int16_t* from, size_t count, float* to);
        void floatToInt16(float* from, size_t count, int16_t* to);
        BufferPool* bufferPool{ nullptr };
    };
}