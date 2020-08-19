#pragma once

#include "AudioPacket.h"
#include "BochanCodec.h"
#include "BufferPool.h"
#include "CodecUtil.h"

namespace bochan {
    class BOCHANAPI AudioCoder {
    public:
        AudioCoder(BufferPool& bufferPool);
        virtual ~AudioCoder() = default;
        virtual void deinitialize() = 0;
        virtual bool isInitialized() const = 0;
        CodecConfig getCodecConfig() const;
    protected:
        ByteBuffer* int16ToFloat(gsl::not_null<ByteBuffer*> samples);
        ByteBuffer* floatToInt16(gsl::not_null<ByteBuffer*> fltp);
        BufferPool* bufferPool{ nullptr };
        CodecConfig config{};
    };
}