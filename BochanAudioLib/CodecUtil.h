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
        static void printDebugInfo(const AVCodecContext* context) {
            BOCHAN_INFO("CONTEXT INFO: \n{} BPCS, {} BPRS, {} FS, {} channels (layout {})\nCodec: {}, {} extradata, {} sample_fmt, {} sample rate",
                        context->bits_per_coded_sample, context->bits_per_raw_sample, context->frame_size,
                        context->channels, context->channel_layout, context->codec_id, context->extradata_size,
                        context->sample_fmt, context->sample_rate);
        }
    };
}

