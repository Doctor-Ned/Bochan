#pragma once

#include "BochanCodec.h"

namespace bochan {
    class BOCHANAPI AudioEncoder {
    public:
        AudioEncoder() = default;
        virtual ~AudioEncoder() = default;
        virtual bool initialize(BochanCodec bochanCodec, int sampleRate, unsigned long long bitRate) = 0;
        virtual void deinitialize() = 0;
        virtual bool isInitialized() = 0;
        virtual BochanCodec getCodec() = 0;
        virtual int getSampleRate() = 0;
        virtual unsigned long long getBitRate() = 0;
    };
}