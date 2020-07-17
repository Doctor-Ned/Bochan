#pragma once
#include <libavcodec\avcodec.h>

class CodecUtil sealed {
public:
    CodecUtil() = delete;
    __declspec(dllexport) static bool isFormatSupported(const AVCodec* codec, const AVSampleFormat format);
    __declspec(dllexport) static int getHighestSupportedSampleRate(const AVCodec* codec);
    static const int STANDARD_SAMPLERATE = 44100;
private:
};

