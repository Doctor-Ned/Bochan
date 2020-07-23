#pragma once

#include "BochanCodec.h"
#include "Buffer.h"

extern "C" {
#include <libavcodec\avcodec.h>
}

#define ERROR_BUFF_SIZE 1024

namespace bochan {
    class BOCHANAPI CodecUtil sealed {
    public:
        static const int STANDARD_SAMPLERATE = 44100;
        static const uint64_t CHANNEL_LAYOUT = AV_CH_LAYOUT_STEREO;
        static const int CHANNELS = 2;
        CodecUtil() = delete;
        static bool isFormatSupported(const AVCodec* codec, const AVSampleFormat format);
        static int getHighestSupportedSampleRate(const AVCodec* codec);
        static AVCodecID getCodecId(const BochanCodec codec);
        static AVSampleFormat getCodecSampleFormat(const BochanCodec codec);
        static void printDebugInfo(const AVCodecContext* context);
        static void int16ToFloat(ByteBuffer* from, float* to);
        static void floatToInt16(ByteBuffer* from, int16_t* to);
        static void int16ToFloat(int16_t* from, size_t count, float* to);
        static void floatToInt16(float* from, size_t count, int16_t* to);
        static inline float int16ToFloat(int16_t value);
        static inline int16_t floatToInt16(float value);
    };
}

