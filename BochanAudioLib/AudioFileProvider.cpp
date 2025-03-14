#include "pch.h"
#include "AudioFileProvider.h"
#include "CodecUtil.h"

bochan::AudioFileProvider::~AudioFileProvider() {
    if (initialized) {
        deinitialize();
    }
}

bool bochan::AudioFileProvider::initialize(gsl::cstring_span fileName, int sampleRate, size_t bufferSize) {
    if (initialized) {
        deinitialize();
    }

    BOCHAN_DEBUG("Initializing with file '{}', {} SR and {} buffer size...", fileName.cbegin(), sampleRate, bufferSize);

    this->fileName = fileName;
    this->sampleRate = sampleRate;

    if (int ret = avformat_open_input(&formatContext, fileName.cbegin(), nullptr, nullptr); ret < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to open file: {}", ret);
        deinitialize();
        return false;
    }

    if (int ret = avformat_find_stream_info(formatContext, NULL); ret < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to find stream info: {}", ret);
        deinitialize();
        return false;
    }

    if (streamId = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0); streamId < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to find audio stream: {}", streamId);
        deinitialize();
        return false;
    }

    context = avcodec_alloc_context3(codec);
    if (!context) {
        BOCHAN_ERROR("Failed to allocate context!");
        deinitialize();
        return false;
    }

    if (int ret = avcodec_parameters_to_context(context, formatContext->streams[streamId]->codecpar); ret < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to get stream parameters: {}", ret);
        deinitialize();
        return false;
    }

    if (int ret = avcodec_open2(context, codec, nullptr); ret < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to open codec: {}", ret);
        deinitialize();
        return false;
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
    resampledFrame = av_frame_alloc();
    if (!frame || !resampledFrame) {
        BOCHAN_ERROR("Failed to allocate frames!");
        deinitialize();
        return false;
    }

    swrContext = swr_alloc_set_opts(nullptr,
                                    av_get_default_channel_layout(CodecUtil::CHANNELS),
                                    AV_SAMPLE_FMT_S16,
                                    sampleRate,
                                    av_get_default_channel_layout(context->channels),
                                    context->sample_fmt,
                                    context->sample_rate,
                                    0, nullptr);
    if (!swrContext) {
        BOCHAN_ERROR("Failed to allocate resampler context!");
        deinitialize();
        return false;
    }

    if (int ret = swr_init(swrContext); ret < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to initialize resampler context: {}", ret);
        deinitialize();
        return false;
    }

    bytesPerSample = av_get_bytes_per_sample(context->sample_fmt);
    internalBuffer = gsl::make_span<uint8_t>(new uint8_t[bufferSize], bufferSize);
    bufferPos = 0ULL;

    initialized = true;

    if (!rewindToStart()) {
        BOCHAN_ERROR("Failed to initially rewind to start: deinitializing!");
        deinitialize();
        return false;
    }
    return true;
}

void bochan::AudioFileProvider::deinitialize() {
    BOCHAN_DEBUG("Deinitializing file provider...");
    initialized = false;
    if (swrContext) {
        swr_free(&swrContext);
    }
    if (resampledFrame) {
        av_frame_free(&resampledFrame);
    }
    if (frame) {
        av_frame_free(&frame);
    }
    if (packet) {
        av_packet_free(&packet);
    }
    if (formatContext) {
        avformat_close_input(&formatContext);
    }
    if (context) {
        avcodec_free_context(&context);
    }
    codec = nullptr;
    streamId = -1;
    fileName = nullptr;
    sampleRate = 0;
    delete[] internalBuffer.begin();
    internalBuffer = {};
    bufferPos = 0ULL;
    bytesPerSample = 0;
}

bool bochan::AudioFileProvider::isInitialized() const {
    return initialized;
}

bool bochan::AudioFileProvider::fillBuffer(gsl::not_null<ByteBuffer*> buff) {
    int samples = static_cast<int>(buff->getUsedSize()) / sizeof(int16_t) / CodecUtil::CHANNELS;
    if (simulateTime) {
        std::chrono::microseconds buffTimeNanos{
            static_cast<long long>(
                1'000'000.0 / static_cast<double>(sampleRate)
                * static_cast<double>(samples)
                ) };
        if (!startPointAvailable) {
            startPointAvailable = true;
            startPoint = std::chrono::system_clock::now();
        }
        startPoint += buffTimeNanos;
        std::this_thread::sleep_until(startPoint);
    }
    uint8_t* ptr = buff->getPointer();
    size_t remaining = buff->getUsedSize();
    while (remaining) {
        if (bufferPos) {
            size_t read = min(remaining, bufferPos);
            memcpy(ptr, internalBuffer.begin(), read);
            remaining -= read;
            ptr += read;
            if (read == bufferPos) {
                bufferPos = 0ULL;
            } else {
                reduceBuffer(read);
            }
        } else {
            std::lock_guard lock(formatMutex);
            if (!readFrame()) {
                if (eof) {
                    BOCHAN_INFO("Reached EOF, returning!");
                } else {
                    BOCHAN_ERROR("Failed to read frame!");
                }
                return false;
            }
        }
    }
    return true;
}

bool bochan::AudioFileProvider::isSimulatingTime() const {
    return simulateTime;
}

void bochan::AudioFileProvider::setSimulateTime(bool simulateTime) {
    this->simulateTime = simulateTime;
    startPointAvailable = false;
}

double bochan::AudioFileProvider::getDuration() {
    if (!initialized) {
        return -1.0;
    }
    return static_cast<double>(formatContext->duration) / AV_TIME_BASE;
}

double bochan::AudioFileProvider::getPositionSeconds() {
    if (!initialized) {
        return -1.0;
    }
    return static_cast<double>(formatContext->streams[streamId]->cur_dts) * formatContext->streams[streamId]->time_base.num / formatContext->streams[streamId]->time_base.den;
}

bool bochan::AudioFileProvider::setPositionSeconds(double position) {
    if (!initialized) {
        return false;
    }
    if (position <= 0.0) {
        return rewindToStart();
    }
    double duration = getDuration();
    if (position > duration) {
        position = duration;
    }
    std::lock_guard lock(formatMutex);
    eof = false;
    int64_t targetPos = static_cast<int64_t>(position * formatContext->streams[streamId]->time_base.den / formatContext->streams[streamId]->time_base.num);
    if (!seekPos(0, targetPos, INT64_MAX)) {
        BOCHAN_ERROR("Failed to seek frame at position {} ({})!", position, targetPos);
        return false;
    }
    return true;
}

bool bochan::AudioFileProvider::rewindForward(double seconds) {
    if (!initialized) {
        return false;
    }
    std::lock_guard lock(formatMutex);
    eof = false;
    int64_t targetPos = formatContext->streams[streamId]->cur_dts + static_cast<int64_t>(seconds * formatContext->streams[streamId]->time_base.den / formatContext->streams[streamId]->time_base.num);
    if (!seekPos(0, targetPos, INT64_MAX)) {
        BOCHAN_ERROR("Failed to seek frame by position {} (to {})!", seconds, targetPos);
        return false;
    }
    return true;
}

bool bochan::AudioFileProvider::rewindBackward(double seconds) {
    return rewindForward(-seconds);
}

bool bochan::AudioFileProvider::rewindToStart() {
    if (!initialized) {
        return false;
    }
    if (!seekPos(0, 0, 0)) {
        BOCHAN_ERROR("Failed to rewind to start!");
        return false;
    }
    return true;
}

bool bochan::AudioFileProvider::isEof() {
    return eof;
}

bool bochan::AudioFileProvider::seekPos(int64_t minPos, int64_t pos, int64_t maxPos) {
    if (!initialized) {
        return false;
    }
    std::lock_guard lock(formatMutex);
    if (int ret = avformat_seek_file(formatContext, streamId, minPos, pos, maxPos, 0); ret < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to seek: {}", ret);
        return false;
    }
    avcodec_flush_buffers(context);
    eof = false;
    return true;
}

bool bochan::AudioFileProvider::readFrame() {
    if (int ret = av_read_frame(formatContext, packet); ret < 0 && packet->buf == nullptr) {
        if (ret == AVERROR_EOF) {
            BOCHAN_DEBUG("Frame read aborted: reached EOF!");
            eof = true;
        } else {
            BOCHAN_LOG_AV_ERROR("Failed to read frame: {}", ret);
        }
        return false;
    }
    if (int ret = avcodec_send_packet(context, packet); ret < 0) {
        BOCHAN_LOG_AV_ERROR("Failed to send packet to decoder: {}", ret);
        return false;
    }
    av_packet_unref(packet);

    while (true) {
        int ret = avcodec_receive_frame(context, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            BOCHAN_LOG_AV_ERROR("Failed to decode frame: {}", ret);
            return false;
        }

        resampledFrame->channel_layout = av_get_default_channel_layout(CodecUtil::CHANNELS);
        resampledFrame->sample_rate = sampleRate;
        resampledFrame->format = AV_SAMPLE_FMT_S16;

        if (frame->channel_layout == 0) {
            frame->channel_layout = av_get_default_channel_layout(frame->channels);
        }

        if (ret = swr_convert_frame(swrContext, resampledFrame, frame); ret < 0) {
            BOCHAN_LOG_AV_ERROR("Failed to resample frame: {}", ret);
        } else {
            size_t remaining = internalBuffer.size_bytes() - bufferPos;
            //size_t linesize = av_samples_get_buffer_size(resampledFrame->linesize, resampledFrame->channels, resampledFrame->nb_samples, static_cast<AVSampleFormat>(resampledFrame->format), 0);
            size_t linesize = sizeof(int16_t) * resampledFrame->nb_samples * resampledFrame->channels;
            if (remaining < linesize) {
                size_t toFree = linesize - remaining;
                BOCHAN_WARN("Buffer overflow! Needed {} bytes, got {}/{}. Truncating {} bytes...", linesize, remaining, internalBuffer.size_bytes(), toFree);
                reduceBuffer(toFree);
            }
            memcpy(internalBuffer.begin() + bufferPos, resampledFrame->extended_data[0], linesize);
            bufferPos += linesize;
        }
        av_frame_unref(resampledFrame);
        av_frame_unref(frame);
    }
    return true;
}

void bochan::AudioFileProvider::reduceBuffer(size_t size) {
    assert(bufferPos >= size);
    if (bufferPos > size) {
        memmove(internalBuffer.begin(), internalBuffer.begin() + size, bufferPos - size);
    }
    bufferPos -= size;
}
