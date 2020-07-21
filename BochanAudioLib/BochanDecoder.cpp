#include "pch.h"
#include "CodecUtil.h"
#include "BochanDecoder.h"

bochan::BochanDecoder::BochanDecoder(BufferPool* bufferPool) : AudioDecoder(bufferPool) {}

bochan::BochanDecoder::~BochanDecoder() {
    deinitialize();
}

bool bochan::BochanDecoder::initialize(BochanCodec bochanCodec, int sampleRate, unsigned long long bitRate) {
    if (initialized) {
        deinitialize();
    }
    BOCHAN_TRACE("Initializing decoder with codec '{}', '{}' sample rate and '{}' bit rate...", bochanCodec, sampleRate, bitRate);
    this->bochanCodec = bochanCodec;
    this->sampleRate = sampleRate;
    this->bitRate = bitRate;
    this->sampleFormat = CodecUtil::getCodecSampleFormat(bochanCodec);
    codecId = CodecUtil::getCodecId(bochanCodec);
    if (codecId == AV_CODEC_ID_NONE) {
        BOCHAN_ERROR("Failed to get codec ID for codec '{}'!", bochanCodec);
        deinitialize();
        return false;
    }
    BOCHAN_TRACE("Using codec ID '{}'...", codecId);
    codec = avcodec_find_decoder(codecId);
    if (!codec) {
        BOCHAN_ERROR("Failed to get decoder for codec ID '{}'!", codecId);
        deinitialize();
        return false;
    }
    BOCHAN_TRACE("Using decoder '{}'...", codec->long_name);
    context = avcodec_alloc_context3(codec);
    if (!context) {
        BOCHAN_ERROR("Failed to allocate context!");
        deinitialize();
        return false;
    }
    context->sample_fmt = sampleFormat;
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
    parser = av_parser_init(codec->id);
    if (!parser) {
        BOCHAN_ERROR("Parser not found!");
        deinitialize();
        return false;
    }
    bytesPerSample = av_get_bytes_per_sample(context->sample_fmt);
    initialized = true;
    return true;
}

void bochan::BochanDecoder::deinitialize() {
    BOCHAN_TRACE("Deinitializing decoder...");
    initialized = false;
    if (parser) {
        av_parser_close(parser);
        parser = nullptr;
    }
    if (frame) {
        av_frame_free(&frame);
    }
    if (packet) {
        av_packet_free(&packet);
    }
    if (context) {
        avcodec_free_context(&context);
    }
    sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_NONE;
    bytesPerSample = 0;
    codec = nullptr;
    codecId = AVCodecID::AV_CODEC_ID_NONE;
    bochanCodec = BochanCodec::None;
    sampleRate = 0;
    bitRate = 0ULL;
}

bool bochan::BochanDecoder::isInitialized() const {
    return initialized;
}

bochan::BochanCodec bochan::BochanDecoder::getCodec() const {
    return bochanCodec;
}

int bochan::BochanDecoder::getSampleRate() const {
    return sampleRate;
}

unsigned long long bochan::BochanDecoder::getBitRate() const {
    return bitRate;
}

std::vector<bochan::ByteBuffer*> bochan::BochanDecoder::decode(ByteBuffer* samples) {
    uint8_t* ptr = samples->getPointer();
    size_t size = samples->getByteSize();
    std::vector<bochan::ByteBuffer*> result;
    while (size) {
        int ret = av_parser_parse2(parser, context, &packet->data, &packet->size,
                                   ptr, size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            char err[ERROR_BUFF_SIZE] = { 0 };
            av_strerror(ret, err, ERROR_BUFF_SIZE);
            BOCHAN_ERROR("Failed to parse data: {}", err);
            //return {};
            break;
        }
        ptr += ret;
        size -= ret;
        if (packet->size) {
            ret = avcodec_send_packet(context, packet);
            if (ret < 0) {
                char err[ERROR_BUFF_SIZE] = { 0 };
                av_strerror(ret, err, ERROR_BUFF_SIZE);
                BOCHAN_ERROR("Failed to send packet to decoder: {}", err);
                return {};
            }
            while (true) {
                ret = avcodec_receive_frame(context, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    char err[ERROR_BUFF_SIZE] = { 0 };
                    av_strerror(ret, err, ERROR_BUFF_SIZE);
                    BOCHAN_ERROR("Failed to decode audio frame: {}", err);
                    return {};
                }
                ByteBuffer* buff =
                    bufferPool->getBuffer(frame->nb_samples * bytesPerSample * context->channels);
                uint8_t* buffPtr = buff->getPointer();
                if (frame->data[1] == nullptr) {
                    memcpy(buffPtr, frame->data[0], buff->getByteSize());
                } else {
                    for (int i = 0; i < frame->nb_samples; ++i) {
                        for (int j = 0; j < context->channels; ++j) {
                            memcpy(buffPtr + (i * context->channels + j) * bytesPerSample, frame->data[j] + bytesPerSample * i, bytesPerSample);
                        }
                    }
                }
                switch (context->sample_fmt) {
                    case AVSampleFormat::AV_SAMPLE_FMT_S16:
                    {
                        result.push_back(buff);
                        break;
                    }
                    case AVSampleFormat::AV_SAMPLE_FMT_FLTP:
                    {
                        ByteBuffer* int16Buff = floatToSamples(buff);
                        result.push_back(int16Buff);
                        bufferPool->freeBuffer(buff);
                        break;
                    }
                }
            }
        } else {
            BOCHAN_TRACE("Encountered an empty packet while decoding!");
        }
    }
    return result;
}
