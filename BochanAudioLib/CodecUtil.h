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
        static const int DEFAULT_SAMPLERATE = 44100;
        static const AVSampleFormat DEFAULT_SAMPLEFORMAT = AVSampleFormat::AV_SAMPLE_FMT_FLTP;
        static const int DEFAULT_FRAMESIZE = 4096;
        static const uint64_t CHANNEL_LAYOUT = AV_CH_LAYOUT_STEREO;
        static const int CHANNELS = 2;
        CodecUtil() = delete;
        static bool isFormatSupported(const AVCodec* codec, const AVSampleFormat format);
        static int getHighestSupportedSampleRate(const AVCodec* codec);
        static std::vector<AVSampleFormat> getSupportedSampleFormats(const AVCodec* codec);
        static std::vector<int> getSupportedSampleRates(const AVCodec* codec);
        static bool isSampleRateSupported(const AVCodec* codec, int sampleRate);
        static AVCodecID getCodecId(const BochanCodec codec);
        static AVSampleFormat getCodecSampleFormat(const BochanCodec codec);
        static void printDebugInfo(const AVCodecContext* context);
        static void int16ToFloat(ByteBuffer* from, float* to);
        static void floatToInt16(ByteBuffer* from, int16_t* to);
        static void int16ToFloat(int16_t* from, size_t count, float* to);
        static void floatToInt16(float* from, size_t count, int16_t* to);
        static float int16ToFloat(int16_t value);
        static int16_t floatToInt16(float value);
    };
}

