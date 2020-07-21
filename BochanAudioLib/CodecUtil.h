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
        static AVSampleFormat getCodecSampleFormat(const BochanCodec codec);
        static const int STANDARD_SAMPLERATE = 44100;
        static const uint64_t CHANNEL_LAYOUT = AV_CH_LAYOUT_STEREO;
        static const int CHANNELS = 2;
    };
}

