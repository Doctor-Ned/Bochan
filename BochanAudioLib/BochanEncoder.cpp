#include "pch.h"
#include "CodecUtil.h"
#include "BochanEncoder.h"

bochan::BochanEncoder::BochanEncoder() {}

bochan::BochanEncoder::~BochanEncoder() {}

bool bochan::BochanEncoder::initialize(BochanCodec bochanCodec) {
    BOCHAN_TRACE("Initializing encoder with codec '{}'...", bochanCodec);
    codecId = CodecUtil::getCodecId(bochanCodec);
    if (codecId == AV_CODEC_ID_NONE) {
        BOCHAN_ERROR("Failed to get codec ID for codec '{}'!", bochanCodec);
        deinitialize();
        return false;
    }
    BOCHAN_TRACE("Using codec ID '{}'...", codecId);
    codec = avcodec_find_encoder(codecId);
    if (!codec) {
        BOCHAN_ERROR("Failed to get encoder for codec ID '{}'!", codecId);
        deinitialize();
        return false;
    }
    BOCHAN_TRACE("Using encoder '{}'...", codec->long_name);
    context = avcodec_alloc_context3(codec);
    if (!context) {
        BOCHAN_ERROR("Failed to allocate context!");
        deinitialize();
        return false;
    }
    // initialize sample rate, format, bitrate etc.
    if (int ret = avcodec_open2(context, codec, nullptr); ret < 0) {
        char err[ERROR_BUFF_SIZE] = { 0 };
        av_strerror(ret, err, ERROR_BUFF_SIZE);
        BOCHAN_ERROR("Failed to open codec: {}", err);
        deinitialize();
        return false;
    }
    packet = av_packet_alloc();
    if (!packet) {
        BOCHAN_ERROR("Failed to allocate packet!");
        deinitialize();
        return false;
    }
    frame = av_frame_alloc();
    if (!frame) {
        BOCHAN_ERROR("Failed to allocate frame!");
        deinitialize();
        return false;
    }
    if (int ret = av_frame_get_buffer(frame, 0); ret < 0) {
        char err[ERROR_BUFF_SIZE] = { 0 };
        av_strerror(ret, err, ERROR_BUFF_SIZE);
        BOCHAN_ERROR("Failed to allocate frame buffer: {}", err);
        deinitialize();
        return false;
    }
    return true;
}

void bochan::BochanEncoder::deinitialize() {
    if (frame) {
        av_frame_free(&frame);
    }
    if (packet) {
        av_packet_free(&packet);
    }
    if (context) {
        avcodec_free_context(&context);
    }
    codec = nullptr;
    codecId = AV_CODEC_ID_NONE;
}
