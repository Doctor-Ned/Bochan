#pragma once

#include "BochanCodec.h"

namespace bochan {
    struct CodecConfig {
        BochanCodec codec{ BochanCodec::None };
        int sampleRate{ 0 };
        unsigned long long bitRate{ 0 };
    };
}