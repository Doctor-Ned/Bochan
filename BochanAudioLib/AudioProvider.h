#pragma once

#include "BufferPool.h"

namespace bochan {
    class BOCHANAPI AudioProvider {
    public:
        AudioProvider() = default;
        virtual ~AudioProvider() = default;
        virtual bool fillBuffer(ByteBuffer* buff) = 0;
    };
}