#pragma once

#include "BufferPool.h"

namespace bochan {
    class BOCHANAPI AudioProvider {
    public:
        AudioProvider() = default;
        virtual ~AudioProvider() = default;
        virtual void deinitialize() = 0;
        virtual bool isInitialized() const = 0;
        virtual bool fillBuffer(ByteBuffer* buff) = 0;
    };
}