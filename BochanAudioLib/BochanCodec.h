#pragma once

#include "pch.h"

namespace bochan {
    enum class BOCHANAPI BochanCodec {
        None,
        MP2,
        FLAC,
        Vorbis,
        AAC,
        Opus
    };
}