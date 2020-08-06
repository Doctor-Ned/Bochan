#pragma once

extern "C" {
#include <libavcodec\avcodec.h>
}

namespace bochan {
    struct AVCodecConfig {
        AVCodecID codecId{ AVCodecID::AV_CODEC_ID_NONE };
        AVSampleFormat sampleFormat{ AVSampleFormat::AV_SAMPLE_FMT_NONE };
        const char* fileName{ nullptr };
    };
}