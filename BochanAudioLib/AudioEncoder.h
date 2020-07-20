#pragma once

#include "AudioCoder.h"

namespace bochan {
    class BOCHANAPI AudioEncoder : public AudioCoder {
    public:
        virtual int getSamplesPerFrame() const = 0;
        virtual std::vector<ByteBuffer*> encode(ByteBuffer* samples) = 0;
    };
}