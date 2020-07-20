#pragma once

#include "BochanCodec.h"

namespace bochan {
    class BOCHANAPI AudioEncoder {
    public:
        AudioEncoder() = default;
        virtual ~AudioEncoder() = default;
        virtual bool initialize(BochanCodec bochanCodec) = 0;
        virtual void deinitialize() = 0;
    };
}