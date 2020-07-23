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
    context->request_sample_fmt = sampleFormat;
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
    //if (context->sample_fmt != sampleFormat) {
    //    BOCHAN_ERROR("Unable to apply requested sample format!");
    //    deinitialize();
    //    return false;
    //}
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
    CodecUtil::printDebugInfo(context);
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
        if (context->extradata) {
            av_free(context->extradata);
            context->extradata = nullptr;
            context->extradata_size = 0;
        }
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

bool bochan::BochanDecoder::needsExtradata() {
    return bochanCodec == BochanCodec::Opus;
}

bool bochan::BochanDecoder::setExtradata(ByteBuffer* extradata) {
    if (initialized) {
        if (context->extradata != nullptr) {
            if (context->extradata_size == extradata->getSize()) {
                memcpy(context->extradata, extradata->getPointer(), context->extradata_size);
                return true;
            } else {
                av_free(context->extradata);
            }
        }
        context->extradata_size = extradata->getSize();
        context->extradata = reinterpret_cast<uint8_t*>(av_malloc(context->extradata_size));
        memcpy(context->extradata, extradata->getPointer(), context->extradata_size);
        return true;
    }
    return false;
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
                    bufferPool->getBuffer(frame->nb_samples * sizeof(uint16_t) * frame->channels);
                uint8_t* buffPtr = buff->getPointer();
                switch (context->sample_fmt) {
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
                        memcpy(buffPtr, frame->data[0], buff->getByteSize());
                        break;
                    }
                    case AVSampleFormat::AV_SAMPLE_FMT_FLTP:
                    {
                        int16_t* int16ptr = reinterpret_cast<int16_t*>(buffPtr);
                        for (int i = 0; i < frame->nb_samples; ++i) {
                            for (int j = 0; j < frame->channels; ++j) {
                                int16ptr[i * frame->channels + j] = static_cast<int16_t>(reinterpret_cast<float*>(frame->data[j])[i] * 32767.9f);
                            }
                        }
                        break;
                    }
                    case AVSampleFormat::AV_SAMPLE_FMT_FLT:
                    {
                        CodecUtil::floatToInt16(reinterpret_cast<float*>(frame->data[0]), frame->nb_samples * frame->channels, reinterpret_cast<int16_t*>(buffPtr));
                        break;
                    }
                    default:
                    {
                        BOCHAN_ERROR("Encountered unsupported decoder format {}!", context->sample_fmt);
                        bufferPool->freeAndRemoveBuffer(buff);
                        return {};
                    }
                }
                result.push_back(buff);
            }
        } else {
            BOCHAN_TRACE("Encountered an empty packet while decoding!");
        }
    }
    return result;
}
