#include "pch.h"
#include "CodecUtil.h"

bool CodecUtil::isFormatSupported(const AVCodec* codec, const AVSampleFormat format) {
    const enum AVSampleFormat* it = codec->sample_fmts;
    while (*it != AV_SAMPLE_FMT_NONE) {
        if (format == *it) {
            return true;
        }
        ++it;
    }
    return false;
}

int CodecUtil::getHighestSupportedSampleRate(const AVCodec* codec) {
    int sampleRate = 0;

    if (!codec->supported_samplerates)
        return STANDARD_SAMPLERATE;

    const int* it = codec->supported_samplerates;
    while (*it) {
        if (!sampleRate || abs(STANDARD_SAMPLERATE - *it) < abs(STANDARD_SAMPLERATE - sampleRate))
            sampleRate = *it;
        ++it;
    }
    return sampleRate;
}
