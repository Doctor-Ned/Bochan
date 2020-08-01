#include "pch.h"
#include "AudioPlayer.h"
#include "CodecUtil.h"

bool bochan::AudioPlayer::initDefault(int sampleRate) {
    this->sampleRate = sampleRate;
    size_t bytesPerSecond = getBytesPerSecond();
    return init(sampleRate, bytesPerSecond / 2ULL, bytesPerSecond * 2ULL);
}

size_t bochan::AudioPlayer::getMinBufferSize() const {
    return minBufferSize;
}

size_t bochan::AudioPlayer::getMaxBufferSize() const {
    return maxBufferSize;
}

size_t bochan::AudioPlayer::getBytesPerSecond() const {
    return static_cast<size_t>(sampleRate) * CodecUtil::CHANNELS * sizeof(int16_t);
}
