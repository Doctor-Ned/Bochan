#pragma once

#include "BufferPool.h"

namespace bochan {
    class BOCHANAPI AudioPlayer {
    public:
        AudioPlayer() = default;
        virtual ~AudioPlayer() = default;
        bool init(int sampleRate);
        virtual bool init(int sampleRate, size_t minBufferSize, size_t maxBufferSize) = 0;
        virtual void deinitialize() = 0;
        virtual bool isInitialized() const = 0;
        virtual size_t queueData(ByteBuffer* buff) = 0;
        virtual bool isPlaying() = 0;
        virtual void play() = 0;
        virtual void stop() = 0;
        virtual void flush() = 0;
    protected:
        int sampleRate{ 0 };
        size_t minBufferSize{ 0ULL }, maxBufferSize{ 0ULL };
    };
}