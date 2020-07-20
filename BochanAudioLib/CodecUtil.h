#pragma once

#include "BochanCodec.h"

extern "C" {
#include <libavcodec\avcodec.h>
}

#define ERROR_BUFF_SIZE 1024

namespace bochan {
    class BOCHANAPI CodecUtil sealed {
    public:
        CodecUtil() = delete;
        static bool isFormatSupported(const AVCodec* codec, const AVSampleFormat format);
        static int getHighestSupportedSampleRate(const AVCodec* codec);
        static AVCodecID getCodecId(const BochanCodec codec);
        static const int STANDARD_SAMPLERATE = 44100;
    };
}

