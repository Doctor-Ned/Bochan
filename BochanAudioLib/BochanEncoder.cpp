#include "pch.h"
#include "BochanEncoder.h"

bochan::BochanEncoder::BochanEncoder(BufferPool& bufferPool) : AudioEncoder(bufferPool) {}

bochan::BochanEncoder::~BochanEncoder() {
    if (initialized) {
        deinitialize();
    }
}

bool bochan::BochanEncoder::initialize(const CodecConfig& config) {
    if (initialized) {
        deinitialize();
    }
    this->config = config;
    avCodecConfig = CodecUtil::getCodecConfig(config.codec);
    BOCHAN_DEBUG("Encoding with codec '{}' at {} SR, {} BPS...", config.codec, config.sampleRate, config.bitRate);
    if (avCodecConfig.codecId == AV_CODEC_ID_NONE) {
        BOCHAN_ERROR("Failed to get codec ID for codec '{}'!", config.codec);
        deinitialize();
        return false;
    }
    BOCHAN_DEBUG("Using codec ID '{}'...", avCodecConfig.codecId);
    codec = avcodec_find_encoder(avCodecConfig.codecId);
    if (!codec) {
        BOCHAN_ERROR("Failed to get encoder for codec ID '{}'!", avCodecConfig.codecId);
        deinitialize();
        return false;
    }
    BOCHAN_DEBUG("Using encoder '{}'...", codec->long_name);
    if (!CodecUtil::isSampleRateSupported(codec, config.sampleRate)) {
        BOCHAN_ERROR("Sample rate {} is not supported by this codec!", config.sampleRate);
        deinitialize();
        return false;
    }
    if (!CodecUtil::isFormatSupported(codec, avCodecConfig.sampleFormat)) {
        BOCHAN_ERROR("Format '{}' is not supported by this codec!", avCodecConfig.sampleFormat);
        deinitialize();
        return false;
    }
    context = avcodec_alloc_context3(codec);
    if (!context) {
        BOCHAN_ERROR("Failed to allocate context!");
        deinitialize();
        return false;
    }
    context->sample_fmt = avCodecConfig.sampleFormat;
    context->bit_rate = config.bitRate;
    context->sample_rate = config.sampleRate;
    context->channel_layout = CodecUtil::CHANNEL_LAYOUT;
    context->channels = CodecUtil::CHANNELS;
    if (int ret = avcodec_open2(context, codec, nullptr); ret < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to open codec: {}", ret);
        deinitialize();
        return false;
    }
    if (!context->frame_size) {
        context->frame_size = CodecUtil::DEFAULT_FRAMESIZE;
        BOCHAN_DEBUG("Unrestricted frame size, setting to {}.", CodecUtil::DEFAULT_FRAMESIZE);
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
    frame->channels = context->channels;
    if (int ret = av_frame_get_buffer(frame, 0); ret < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to allocate frame buffer: {}", ret);
        deinitialize();
        return false;
    }
    bytesPerSample = av_get_bytes_per_sample(context->sample_fmt);
    CodecUtil::printDebugInfo(context);
    initialized = true;
    return true;
}

void bochan::BochanEncoder::deinitialize() {
    BOCHAN_DEBUG("Deinitializing encoder...");
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
    pts = 0;
    bytesPerSample = 0;
    codec = nullptr;
    avCodecConfig = {};
    config = {};
}

bool bochan::BochanEncoder::isInitialized() const {
    return initialized;
}

int bochan::BochanEncoder::getSamplesPerFrame() const {
    return initialized ? context->frame_size : 0;
}

int bochan::BochanEncoder::getInputBufferByteSize() const {
    return initialized ? context->frame_size * sizeof(uint16_t) * context->channels : 0;
}

bool bochan::BochanEncoder::hasExtradata() {
    return initialized && context->extradata != nullptr;
}

bochan::ByteBuffer* bochan::BochanEncoder::getExtradata() {
    if (!hasExtradata()) {
        return nullptr;
    }
    ByteBuffer* result = bufferPool->getBuffer(context->extradata_size);
    memcpy(result->getPointer(), context->extradata, context->extradata_size);
    return result;
}

std::vector<bochan::ByteBuffer*> bochan::BochanEncoder::encode(ByteBuffer* samples) {
    assert(samples->getUsedSize() == getInputBufferByteSize());
    if (int ret = av_frame_make_writable(frame); ret < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to ensure writable frame: {}", ret);
        return {};
    }
    size_t expectedSamples = static_cast<size_t>(frame->nb_samples * frame->channels);
    size_t providedSamples = samples->getUsedSize() / sizeof(uint16_t);
    if (providedSamples != expectedSamples) {
        BOCHAN_ERROR("Failed to encode audio frame! Expected {} samples, got {}.",
                     expectedSamples, providedSamples);
        return {};
    }
    frame->pts = pts;
    pts += frame->nb_samples;
    switch (frame->format) {
        case AVSampleFormat::AV_SAMPLE_FMT_S16P:
        {
            uint16_t* uint16ptr = reinterpret_cast<uint16_t*>(samples->getPointer());
            for (int i = 0; i < frame->nb_samples; ++i) {
                for (int j = 0; j < frame->channels; ++j) {
                    reinterpret_cast<uint16_t*>(frame->data[j])[i] = uint16ptr[i * frame->channels + j];
                }
            }
            break;
        }
        case AVSampleFormat::AV_SAMPLE_FMT_S16:
        {
            memcpy(frame->data[0], samples, samples->getUsedSize());
            break;
        }
        case AVSampleFormat::AV_SAMPLE_FMT_FLTP:
        {
            int16_t* int16ptr = reinterpret_cast<int16_t*>(samples->getPointer());
            for (int i = 0; i < frame->nb_samples; ++i) {
                for (int j = 0; j < frame->channels; ++j) {
                    reinterpret_cast<float*>(frame->data[j])[i] = CodecUtil::int16ToFloat(int16ptr[i * frame->channels + j]);
                }
            }
            break;
        }
        case AVSampleFormat::AV_SAMPLE_FMT_FLT:
        {
            CodecUtil::int16ToFloat(samples, reinterpret_cast<float*>(frame->data[0]));
            break;
        }
        default:
        {
            BOCHAN_ERROR("Encountered unsupported decoder format {}!", context->sample_fmt);
            return {};
        }
    }
    if (int ret = avcodec_send_frame(context, frame); ret < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to send frame to encoder: {}", ret);
        return {};
    }
    std::vector<ByteBuffer*> result;
    while (true) {
        int ret = avcodec_receive_packet(context, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            BOCHAN_LOG_AV_ERROR("Failed to encode audio frame: {}", ret);
            for (ByteBuffer* buff : result) {
                if (!bufferPool->freeAndRemoveBuffer(buff)) {
                    BOCHAN_WARN("Failed to free and remove the sample buffer!");
                }
            }
            return {};
        }
        ByteBuffer* buff = bufferPool->getBuffer(packet->size);
        memcpy(buff->getPointer(), packet->data, packet->size);
        result.push_back(buff);
    }
    return result;
}
