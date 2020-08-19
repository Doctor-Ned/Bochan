#include "pch.h"
#include "CodecUtil.h"

#include <stdexcept>

std::map<bochan::BochanCodec, bochan::AVCodecConfig> bochan::CodecUtil::codecAvConfigMap{
    {BochanCodec::Opus, {AV_CODEC_ID_OPUS, AV_SAMPLE_FMT_FLT, "recording.opus"}},
    {BochanCodec::Vorbis, {AV_CODEC_ID_VORBIS, AV_SAMPLE_FMT_FLTP, "recording.ogg"}}
};

static void avLogCallback(void* ptr, int level, const char* szFmt, va_list varg) {
    va_list vl;
    static const char* logFormat{ "[libav]{}" };
    char line[1024];
    static int printPrefix = 1;
    va_copy(vl, varg);
    av_log_format_line(ptr, level, szFmt, vl, line, sizeof(line), &printPrefix);
    va_end(vl);

    switch (level) {
        case AV_LOG_TRACE:
            BOCHAN_TRACE(logFormat, line);
            break;
        case AV_LOG_DEBUG:
        case AV_LOG_VERBOSE:
            BOCHAN_DEBUG(logFormat, line);
            break;
        case AV_LOG_INFO:
            BOCHAN_INFO(logFormat, line);
            break;
        case AV_LOG_WARNING:
            BOCHAN_WARN(logFormat, line);
            break;
        case AV_LOG_ERROR:
            BOCHAN_ERROR(logFormat, line);
            break;
        case AV_LOG_FATAL:
        case AV_LOG_PANIC:
            BOCHAN_CRITICAL(logFormat, line);
            break;
    }
}

void bochan::CodecUtil::initialiseAvLog() {
    av_log_set_callback(avLogCallback);
    switch (BOCHAN_LOG_LEVEL) {
        case BOCHAN_LEVEL_TRACE:
            av_log_set_level(AV_LOG_TRACE);
            break;
        case BOCHAN_LEVEL_DEBUG:
            av_log_set_level(AV_LOG_DEBUG);
            break;
        case BOCHAN_LEVEL_INFO:
            av_log_set_level(AV_LOG_INFO);
            break;
        case BOCHAN_LEVEL_WARN:
            av_log_set_level(AV_LOG_WARNING);
            break;
        case BOCHAN_LEVEL_ERROR:
            av_log_set_level(AV_LOG_ERROR);
            break;
        case BOCHAN_LEVEL_CRITICAL:
            av_log_set_level(AV_LOG_FATAL);
            break;
        case BOCHAN_LEVEL_OFF:
            av_log_set_level(AV_LOG_QUIET);
            break;
    }
}

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
        if (*it > sampleRate) {
            sampleRate = *it;
        }
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

std::vector<int> bochan::CodecUtil::getSupportedSampleRates(const BochanCodec codec) {
    AVCodec* avCodec = avcodec_find_encoder(getCodecConfig(codec).codecId);
    if (!avCodec) {
        return {};
    }
    return getSupportedSampleRates(avCodec);
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

bochan::AVCodecConfig bochan::CodecUtil::getCodecConfig(const BochanCodec codec) {
    std::map<BochanCodec, AVCodecConfig>::iterator it = codecAvConfigMap.find(codec);
    if (it == codecAvConfigMap.end()) {
        return {};
    }
    return it->second;
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
    int16ToFloat(reinterpret_cast<int16_t*>(from->getPointer()), from->getUsedSize() / sizeof(int16_t), to);
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

size_t bochan::CodecUtil::getBytesPerSecond(int sampleRate) {
    return static_cast<size_t>(sampleRate) * CHANNELS * sizeof(int16_t);
}
