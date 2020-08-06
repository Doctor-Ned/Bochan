#pragma once

#include "CodecConfig.h"
#include "AVCodecConfig.h"
#include "Buffer.h"

#include <map>

#define ERROR_BUFF_SIZE 1024

#define BOCHAN_LOG_AV_ERROR(format, errorCode)          \
do {                                                    \
    char errorBuff[ERROR_BUFF_SIZE] = { 0 };            \
    av_strerror(errorCode, errorBuff, ERROR_BUFF_SIZE); \
    BOCHAN_ERROR(format, errorBuff);                    \
} while (false)

namespace bochan {
    class CodecUtil sealed {
    public:
        static const int DEFAULT_SAMPLERATE = 44100;
        static const AVSampleFormat DEFAULT_SAMPLEFORMAT = AVSampleFormat::AV_SAMPLE_FMT_FLTP;
        static const int DEFAULT_FRAMESIZE = 4096;
        static const uint64_t CHANNEL_LAYOUT = AV_CH_LAYOUT_STEREO;
        static const int CHANNELS = 2;
        CodecUtil() = delete;
        BOCHANAPI static void initialiseAvLog();
        BOCHANAPI static bool isFormatSupported(const AVCodec* codec, const AVSampleFormat format);
        BOCHANAPI static int getHighestSupportedSampleRate(const AVCodec* codec);
        BOCHANAPI static std::vector<AVSampleFormat> getSupportedSampleFormats(const AVCodec* codec);
        BOCHANAPI static std::vector<int> getSupportedSampleRates(const AVCodec* codec);
        BOCHANAPI static bool isSampleRateSupported(const AVCodec* codec, int sampleRate);
        BOCHANAPI static AVCodecConfig getCodecConfig(const BochanCodec codec);
        BOCHANAPI static void printDebugInfo(const AVCodecContext* context);
        BOCHANAPI static void int16ToFloat(ByteBuffer* from, float* to);
        BOCHANAPI static void floatToInt16(ByteBuffer* from, int16_t* to);
        BOCHANAPI static void int16ToFloat(int16_t* from, size_t count, float* to);
        BOCHANAPI static void floatToInt16(float* from, size_t count, int16_t* to);
        BOCHANAPI static float int16ToFloat(int16_t value);
        BOCHANAPI static int16_t floatToInt16(float value);
    private:
        static std::map<BochanCodec, AVCodecConfig> codecAvConfigMap;
    };
}

