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
        static const int CHANNELS = 2;
        CodecUtil() = delete;
        BOCHANAPI static void initialiseAvLog();
        BOCHANAPI static bool isFormatSupported(const gsl::not_null<AVCodec*> codec, const AVSampleFormat format);
        BOCHANAPI static int getHighestSupportedSampleRate(const gsl::not_null<AVCodec*> codec);
        BOCHANAPI static std::vector<AVSampleFormat> getSupportedSampleFormats(const gsl::not_null<AVCodec*> codec);
        BOCHANAPI static std::vector<int> getSupportedSampleRates(const gsl::not_null<AVCodec*> codec);
        BOCHANAPI static std::vector<int> getSupportedSampleRates(const BochanCodec codec);
        BOCHANAPI static bool isSampleRateSupported(const gsl::not_null<AVCodec*> codec, int sampleRate);
        BOCHANAPI static AVCodecConfig getCodecConfig(const BochanCodec codec);
        BOCHANAPI static void printDebugInfo(const gsl::not_null<AVCodecContext*> context);
        BOCHANAPI static void int16ToFloat(gsl::span<int16_t> from, gsl::span<float> to);
        BOCHANAPI static void floatToInt16(gsl::span<float> from, gsl::span<int16_t> to);
        BOCHANAPI static float int16ToFloat(int16_t value);
        BOCHANAPI static int16_t floatToInt16(float value);
        BOCHANAPI static size_t getBytesPerSecond(int sampleRate);
    private:
        static std::map<BochanCodec, AVCodecConfig> codecAvConfigMap;
    };
}

