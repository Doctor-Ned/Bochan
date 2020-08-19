#include "pch.h"
#include "BochanDecoder.h"

extern "C" {
#include <libavutil/avstring.h>
}

bochan::BochanDecoder::BochanDecoder(BufferPool& bufferPool) : AudioDecoder(bufferPool) {}

bochan::BochanDecoder::~BochanDecoder() {
    if (initialized) {
        deinitialize();
    }
}

bool bochan::BochanDecoder::initialize(const CodecConfig& config, bool saveToFile, ByteBuffer* extradata) {
    if (initialized) {
        deinitialize();
    }
    this->config = config;
    this->saveToFile = saveToFile;
    avCodecConfig = CodecUtil::getCodecConfig(config.codec);
    BOCHAN_DEBUG("Decoding with codec '{}' at {} SR, {} BPS...", config.codec, config.sampleRate, config.bitRate);
    if (needsExtradata(config.codec)) {
        if (extradata == nullptr) {
            BOCHAN_ERROR("Extradata required but not provided!");
            return false;
        }
    } else if (extradata != nullptr) {
        BOCHAN_WARN("Extradata provided, but not marked as required.");
    }
    if (avCodecConfig.codecId == AV_CODEC_ID_NONE) {
        BOCHAN_ERROR("Failed to get codec ID for codec '{}'!", config.codec);
        deinitialize();
        return false;
    }
    BOCHAN_DEBUG("Using codec ID '{}'...", avCodecConfig.codecId);
    codec = avcodec_find_decoder(avCodecConfig.codecId);
    if (!codec) {
        BOCHAN_ERROR("Failed to get decoder for codec ID '{}'!", avCodecConfig.codecId);
        deinitialize();
        return false;
    }
    BOCHAN_DEBUG("Using decoder '{}'...", codec->long_name);
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
    formatContext = avformat_alloc_context();
    if (!formatContext) {
        BOCHAN_ERROR("Failed to allocate format context!");
        deinitialize();
        return false;
    }
    if (saveToFile) {
        if (int ret = avio_open(&avioContext, avCodecConfig.fileName, AVIO_FLAG_WRITE); ret < 0) {
            BOCHAN_LOG_AV_ERROR("Failed to open avio context: {}", ret);
            deinitialize();
            return false;
        }
        formatContext->pb = avioContext;
        formatContext->url = reinterpret_cast<char*>(av_malloc(sizeof(avCodecConfig.fileName)));
        memcpy(formatContext->url, avCodecConfig.fileName, sizeof(avCodecConfig.fileName));
    }
    formatContext->oformat = av_guess_format(nullptr, avCodecConfig.fileName, nullptr);
    if (!formatContext->oformat) {
        BOCHAN_ERROR("Failed to find output file format!");
        deinitialize();
        return false;
    }
    if (!saveToFile) {
        formatContext->oformat->flags |= AVFMT_NOFILE;
    }
    stream = avformat_new_stream(formatContext, nullptr);
    context = avcodec_alloc_context3(codec);
    if (!context) {
        BOCHAN_ERROR("Failed to allocate context!");
        deinitialize();
        return false;
    }
    context->request_sample_fmt = avCodecConfig.sampleFormat;
    context->bit_rate = config.bitRate;
    context->sample_rate = config.sampleRate;
    context->channel_layout = av_get_default_channel_layout(CodecUtil::CHANNELS);
    context->channels = CodecUtil::CHANNELS;
    stream->time_base.den = context->sample_rate;
    stream->time_base.num = 1;
    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    if (extradata) {
        context->extradata_size = static_cast<int>(extradata->getUsedSize());
        context->extradata = reinterpret_cast<uint8_t*>(av_malloc(context->extradata_size));
        memcpy(context->extradata, extradata->getPointer(), context->extradata_size);
        if (!bufferPool->freeAndRemoveBuffer(extradata)) {
            BOCHAN_WARN("Failed to free and remove the extradata buffer!");
        }
    }
    if (int ret = avcodec_open2(context, codec, nullptr); ret < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to open codec: {}", ret);
        deinitialize();
        return false;
    }
    if (context->sample_rate != config.sampleRate) {
        BOCHAN_ERROR("Sample rate {} is unsupported for this codec (changed to {})!", config.sampleRate, context->sample_rate);
        deinitialize();
        return false;
    }
    if (int ret = avcodec_parameters_from_context(stream->codecpar, context); ret < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to initialize stream parameters: {}", ret);
        deinitialize();
        return false;
    }
    if (saveToFile) {
        if (int ret = avformat_write_header(formatContext, nullptr); ret < 0) {
            BOCHAN_LOG_AV_ERROR("Failed to write file header: {}", ret);
            deinitialize();
            return false;
        }
    }
    packet = av_packet_alloc();
    if (!packet) {
        BOCHAN_ERROR("Failed to allocate packet!");
        deinitialize();
        return false;
    }
    packet->data = nullptr;
    packet->size = 0;
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
    parser = av_parser_init(codec->id);
    if (!parser) {
        BOCHAN_ERROR("Parser not found!");
        deinitialize();
        return false;
    }
    bytesPerSample = av_get_bytes_per_sample(context->sample_fmt);
    CodecUtil::printDebugInfo(context);
    initialized = true;
    return true;
}

void bochan::BochanDecoder::deinitialize() {
    BOCHAN_DEBUG("Deinitializing decoder...");
    bool wasInitialized = initialized;
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
    if (formatContext) {
        if (wasInitialized && saveToFile) {
            if (int ret = av_write_frame(formatContext, nullptr); ret < 0) {
                BOCHAN_LOG_AV_ERROR("Failed to flush frame to the file: {}", ret);
            }
        }
        avformat_free_context(formatContext);
    }
    if (avioContext) {
        if (int ret = avio_close(avioContext); ret < 0) {
            BOCHAN_LOG_AV_ERROR("Failed to close AVIO context: {}", ret);
        }
    }
    if (context) {
        if (context->extradata) {
            av_free(context->extradata);
            context->extradata = nullptr;
            context->extradata_size = 0;
        }
        avcodec_free_context(&context);
    }
    saveToFile = false;
    bytesPerSample = 0;
    codec = nullptr;
    avCodecConfig = {};
    config = {};
}

bool bochan::BochanDecoder::isInitialized() const {
    return initialized;
}

bool bochan::BochanDecoder::needsExtradata(BochanCodec bochanCodec) {
    switch (bochanCodec) {
        default:
            return false;
        case BochanCodec::Vorbis:
            [[fallthrough]];
        case BochanCodec::Opus:
            return true;
    }
}

std::vector<bochan::ByteBuffer*> bochan::BochanDecoder::decode(AudioPacket audioPacket) {
    uint8_t* ptr = audioPacket.buffer->getPointer();
    size_t size = audioPacket.buffer->getUsedSize();
    std::vector<bochan::ByteBuffer*> result;
    while (size) {
        int ret{};
        ret = av_parser_parse2(parser, context, &packet->data, &packet->size,
                               ptr, static_cast<int>(size), audioPacket.pts, audioPacket.pts, 0);
        if (ret < 0) {
            BOCHAN_LOG_AV_ERROR("Failed to parse data: {}", ret);
            break;
        }
        ptr += ret;
        size -= ret;
        packet->pts = audioPacket.pts;
        packet->dts = audioPacket.pts;
        if (packet->size) {
            if (saveToFile) {
                if (ret = av_write_frame(formatContext, packet); ret < 0) {
                    BOCHAN_LOG_AV_ERROR("Failed to write frame to the file: {}", ret);
                }
            }
            if (ret = avcodec_send_packet(context, packet); ret < 0) {
                BOCHAN_LOG_AV_ERROR("Failed to send packet to decoder: {}", ret);
                return {};
            }
            while (true) {
                if (ret = avcodec_receive_frame(context, frame); ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }
                if (ret < 0) {
                    BOCHAN_LOG_AV_ERROR("Failed to decode audio frame: {}", ret);
                    for (ByteBuffer* buff : result) {
                        if (!bufferPool->freeAndRemoveBuffer(buff)) {
                            BOCHAN_WARN("Failed to free and remove the sample buffer!");
                        }
                    }
                    return {};
                }
                ByteBuffer* buff =
                    bufferPool->getBuffer(frame->nb_samples * sizeof(uint16_t) * frame->channels);
                uint8_t* buffPtr = buff->getPointer();
                result.push_back(buff);
                switch (frame->format) {
                    case AVSampleFormat::AV_SAMPLE_FMT_S16P:
                    {
                        uint16_t* uint16ptr = reinterpret_cast<uint16_t*>(buffPtr);
                        for (int i = 0; i < frame->nb_samples; ++i) {
                            for (int j = 0; j < frame->channels; ++j) {
                                uint16ptr[i * frame->channels + j] = reinterpret_cast<uint16_t*>(frame->data[j])[i];
                            }
                        }
                        break;
                    }
                    case AVSampleFormat::AV_SAMPLE_FMT_S16:
                    {
                        memcpy(buffPtr, frame->data[0], buff->getUsedSize());
                        break;
                    }
                    case AVSampleFormat::AV_SAMPLE_FMT_FLTP:
                    {
                        int16_t* int16ptr = reinterpret_cast<int16_t*>(buffPtr);
                        for (int i = 0; i < frame->nb_samples; ++i) {
                            for (int j = 0; j < frame->channels; ++j) {
                                int16ptr[i * frame->channels + j] = CodecUtil::floatToInt16(reinterpret_cast<float*>(frame->data[j])[i]);
                            }
                        }
                        break;
                    }
                    case AVSampleFormat::AV_SAMPLE_FMT_FLT:
                    {
                        CodecUtil::floatToInt16(reinterpret_cast<float*>(frame->data[0]), static_cast<size_t>(frame->nb_samples) * frame->channels, reinterpret_cast<int16_t*>(buffPtr));
                        break;
                    }
                    default:
                    {
                        BOCHAN_ERROR("Encountered unsupported decoder format {}!", context->sample_fmt);
                        for (ByteBuffer* buff : result) {
                            if (!bufferPool->freeAndRemoveBuffer(buff)) {
                                BOCHAN_WARN("Failed to free and remove the sample buffer!");
                            }
                        }
                        return {};
                    }
                }
            }
        } else {
            BOCHAN_DEBUG("Encountered an empty packet while decoding!");
        }
    }
    return result;
}
