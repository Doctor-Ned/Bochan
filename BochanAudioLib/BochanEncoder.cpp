#include "pch.h"
#include "CodecUtil.h"
#include "BochanEncoder.h"

bochan::BochanEncoder::BochanEncoder(BufferPool* bufferPool) : bufferPool(bufferPool) {}

bochan::BochanEncoder::~BochanEncoder() {
    deinitialize();
}

bool bochan::BochanEncoder::initialize(BochanCodec bochanCodec, int sampleRate, unsigned long long bitRate) {
    if (initialized) {
        deinitialize();
    }
    BOCHAN_TRACE("Initializing encoder with codec '{}', '{}' sample rate and '{}' bit rate...", bochanCodec, sampleRate, bitRate);
    this->bochanCodec = bochanCodec;
    this->sampleRate = sampleRate;
    this->bitRate = bitRate;
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
    context->sample_fmt =
        bochanCodec == BochanCodec::AAC
        ? AVSampleFormat::AV_SAMPLE_FMT_FLTP
        : CodecUtil::SAMPLE_FORMAT;
    if (!CodecUtil::isFormatSupported(codec, context->sample_fmt)) {
        BOCHAN_ERROR("Format '{}' is not supported by codec '{}'!", context->sample_fmt, codec->long_name);
        deinitialize();
        return false;
    }
    context->bit_rate = bitRate;
    context->sample_rate = sampleRate;
    context->channel_layout = CodecUtil::CHANNEL_LAYOUT;
    context->channels = CodecUtil::CHANNELS;
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
    frame->nb_samples = context->frame_size;
    frame->format = context->sample_fmt;
    frame->channel_layout = context->channel_layout;
    if (int ret = av_frame_get_buffer(frame, 0); ret < 0) {
        char err[ERROR_BUFF_SIZE] = { 0 };
        av_strerror(ret, err, ERROR_BUFF_SIZE);
        BOCHAN_ERROR("Failed to allocate frame buffer: {}", err);
        deinitialize();
        return false;
    }
    initialized = true;
    return true;
}

void bochan::BochanEncoder::deinitialize() {
    BOCHAN_TRACE("Deinitializing encoder...");
    initialized = false;
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
    bochanCodec = BochanCodec::None;
    sampleRate = 0;
    bitRate = 0ULL;
}

bool bochan::BochanEncoder::isInitialized() const {
    return initialized;
}

bochan::BochanCodec bochan::BochanEncoder::getCodec() const {
    return bochanCodec;
}

int bochan::BochanEncoder::getSampleRate() const {
    return sampleRate;
}

unsigned long long bochan::BochanEncoder::getBitRate() const {
    return bitRate;
}

int bochan::BochanEncoder::getSamplesPerFrame() const {
    return initialized ? context->frame_size : 0;
}

std::vector<bochan::ByteBuffer*> bochan::BochanEncoder::encode(Buffer<uint16_t>* samples) {
    if (int ret = av_frame_make_writable(frame); ret < 0) {
        char err[ERROR_BUFF_SIZE] = { 0 };
        av_strerror(ret, err, ERROR_BUFF_SIZE);
        BOCHAN_ERROR("Failed to ensure writable frame: {}", err);
        return {};
    }
    if (samples->getSize() != context->frame_size) {
        BOCHAN_ERROR("Failed to encode audio frame! Expected {} samples, got {}.", context->frame_size, samples->getSize());
        return {};
    }
    memcpy(frame->data[0], samples, samples->getByteSize());
    if (int ret = avcodec_send_frame(context, frame); ret < 0) {
        char err[ERROR_BUFF_SIZE] = { 0 };
        av_strerror(ret, err, ERROR_BUFF_SIZE);
        BOCHAN_ERROR("Failed to send frame to encoder: {}", err);
        return {};
    }
    std::vector<ByteBuffer*> result;
    while (true) {
        int ret = avcodec_receive_packet(context, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            char err[ERROR_BUFF_SIZE] = { 0 };
            av_strerror(ret, err, ERROR_BUFF_SIZE);
            BOCHAN_ERROR("Failed to encode audio frame: {}", err);
            return {};
        }
        ByteBuffer* buff = bufferPool->getBuffer(packet->size);
        memcpy(buff->getPointer(), packet->data, packet->size);
        result.push_back(buff);
        av_packet_unref(packet);
    }
    return result;
}
