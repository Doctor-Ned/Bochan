#pragma once

#include "AudioEncoder.h"

extern "C" {
#include <libavformat\avformat.h>
}

namespace bochan {
    class BOCHANAPI BochanEncoder : public AudioEncoder {
    public:
        BochanEncoder();
        ~BochanEncoder();
        bool initialize(BochanCodec bochanCodec) override;
        void deinitialize() override;
    private:
        AVCodecID codecId{};
        AVCodec* codec{ nullptr };
        AVCodecContext* context{ nullptr };
        AVPacket* packet{ nullptr };
        AVFrame* frame{ nullptr };
    };
}