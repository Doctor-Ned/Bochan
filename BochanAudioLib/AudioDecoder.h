#pragma once

#include "AudioCoder.h"

namespace bochan {
    class BOCHANAPI AudioDecoder : public AudioCoder {
    public:
        virtual std::vector<ByteBuffer*> decode(ByteBuffer* samples) = 0;
    };
}