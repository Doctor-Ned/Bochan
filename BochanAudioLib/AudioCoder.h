#pragma once

#include "BochanCodec.h"
#include "BufferPool.h"

namespace bochan {
    class BOCHANAPI AudioCoder {
    public:
        AudioCoder(BufferPool& bufferPool);
        virtual ~AudioCoder() = default;
        virtual void deinitialize() = 0;
        virtual bool isInitialized() const = 0;
        virtual BochanCodec getCodec() const = 0;
        virtual int getSampleRate() const = 0;
        virtual unsigned long long getBitRate() const = 0;
    protected:
        ByteBuffer* int16ToFloat(ByteBuffer* samples);
        ByteBuffer* floatToInt16(ByteBuffer* fltp);
        BufferPool* bufferPool{ nullptr };
        BochanCodec bochanCodec{ BochanCodec::None };
        int sampleRate{ 0 };
        unsigned long long bitRate{ 0ULL };
    };
}