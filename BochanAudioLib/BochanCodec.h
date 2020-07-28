#pragma once

#include "pch.h"

namespace bochan {
    enum class BOCHANAPI BochanCodec {
        None,
        WAV,
        FLAC,
        Vorbis,
        AAC,
        Opus
    };
}