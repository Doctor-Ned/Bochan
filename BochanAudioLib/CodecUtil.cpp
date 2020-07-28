#include "pch.h"
#include "CodecUtil.h"

#include <stdexcept>

bool bochan::CodecUtil::isFormatSupported(const AVCodec* codec, const AVSampleFormat format) {
    if (!codec->sample_fmts) {
        return DEFAULT_SAMPLEFORMAT;
    }
    const enum AVSampleFormat* it = codec->sample_fmts;
    while (*it != AV_SAMPLE_FMT_NONE) {
        if (format == *it) {
            return true;
        }
        ++it;
    }
    return false;
}

int bochan::CodecUtil::getHighestSupportedSampleRate(const AVCodec* codec) {
    if (!codec->supported_samplerates)
        return DEFAULT_SAMPLERATE;

    int sampleRate = 0;
    const int* it = codec->supported_samplerates;
    while (*it) {
        if (!sampleRate || abs(DEFAULT_SAMPLERATE - *it) < abs(DEFAULT_SAMPLERATE - sampleRate))
            sampleRate = *it;
        ++it;
    }
    return sampleRate;
}

std::vector<AVSampleFormat> bochan::CodecUtil::getSupportedSampleFormats(const AVCodec* codec) {
    if (!codec->sample_fmts) {
        return {};
    }
    const enum AVSampleFormat* it = codec->sample_fmts;
    std::vector<AVSampleFormat> result;
    while (*it != AV_SAMPLE_FMT_NONE) {
        result.push_back(*it);
        ++it;
    }
    return result;
}

std::vector<int> bochan::CodecUtil::getSupportedSampleRates(const AVCodec* codec) {
    if (!codec->supported_samplerates)
        return {};

    int sampleRate = 0;
    const int* it = codec->supported_samplerates;
    std::vector<int> result;
    while (*it) {
        result.push_back(*it);
        ++it;
    }
    return result;
}

bool bochan::CodecUtil::isSampleRateSupported(const AVCodec* codec, int sampleRate) {
    if (sampleRate <= 0) {
        return false;
    }
    if (!codec->supported_samplerates) {
        return true;
    }
    const int* it = codec->supported_samplerates;
    while (*it) {
        if (*it == sampleRate) {
            return true;
        }
        ++it;
    }
    return false;
}

AVCodecID bochan::CodecUtil::getCodecId(const BochanCodec codec) {
    switch (codec) {
        default:
            return AVCodecID::AV_CODEC_ID_NONE;
        case BochanCodec::WAV:
            return AVCodecID::AV_CODEC_ID_PCM_S16LE;
        case BochanCodec::FLAC:
            return AVCodecID::AV_CODEC_ID_FLAC;
        case BochanCodec::Vorbis:
            return AVCodecID::AV_CODEC_ID_VORBIS;
        case BochanCodec::AAC:
            return AVCodecID::AV_CODEC_ID_AAC;
        case BochanCodec::Opus:
            return AVCodecID::AV_CODEC_ID_OPUS;
    }
}

AVSampleFormat bochan::CodecUtil::getCodecSampleFormat(const BochanCodec codec) {
    switch (codec) {
        default:
            return AVSampleFormat::AV_SAMPLE_FMT_NONE;
        case BochanCodec::WAV:
            return AVSampleFormat::AV_SAMPLE_FMT_S16;
        case BochanCodec::FLAC:
            return AVSampleFormat::AV_SAMPLE_FMT_S16;
        case BochanCodec::Vorbis:
            return AVSampleFormat::AV_SAMPLE_FMT_FLTP;
        case BochanCodec::AAC:
            return AVSampleFormat::AV_SAMPLE_FMT_FLTP;
        case BochanCodec::Opus:
            return AVSampleFormat::AV_SAMPLE_FMT_FLT;
    }
}

void bochan::CodecUtil::printDebugInfo(const AVCodecContext* context) {
    BOCHAN_INFO("CONTEXT INFO:");
    BOCHAN_INFO("{} BPCS, {} BPRS, {} FS, {} channels (layout {})",
                context->bits_per_coded_sample, context->bits_per_raw_sample, context->frame_size,
                context->channels, context->channel_layout);
    BOCHAN_INFO("Codec: {}, {} extradata, format {}, {} sample rate", context->codec_id, context->extradata_size,
                context->sample_fmt, context->sample_rate);
}

void bochan::CodecUtil::int16ToFloat(ByteBuffer* from, float* to) {
    int16ToFloat(reinterpret_cast<int16_t*>(from->getPointer()), from->getUsedSize() / 2ULL, to);
}

void bochan::CodecUtil::floatToInt16(ByteBuffer* from, int16_t* to) {
    floatToInt16(reinterpret_cast<float*>(from->getPointer()), from->getUsedSize() / sizeof(float), to);
}

void bochan::CodecUtil::int16ToFloat(int16_t* from, size_t count, float* to) {
    for (int i = 0; i < count; ++i) {
        to[i] = int16ToFloat(from[i]);
    }
}

void bochan::CodecUtil::floatToInt16(float* from, size_t count, int16_t* to) {
    for (int i = 0; i < count; ++i) {
        to[i] = floatToInt16(from[i]);
    }
}

float bochan::CodecUtil::int16ToFloat(int16_t value) {
    return static_cast<float>(value) / 32768.0f;
}

int16_t bochan::CodecUtil::floatToInt16(float value) {
    return static_cast<int16_t>(value * 32767.9f);
}
