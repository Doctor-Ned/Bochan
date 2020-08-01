#pragma once

#include "BufferPool.h"

namespace bochan {
    class BOCHANAPI AudioProvider {
    public:
        AudioProvider() = default;
        virtual ~AudioProvider() = default;
        virtual bool init(int sampleRate) = 0;
        virtual void deinitialize() = 0;
        virtual bool isInitialized() const = 0;
        virtual bool fillBuffer(ByteBuffer* buff) = 0;
    protected:
        int sampleRate{ 0 };
    };
}