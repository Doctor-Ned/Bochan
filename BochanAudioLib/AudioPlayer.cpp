#include "pch.h"
#include "AudioPlayer.h"
#include "CodecUtil.h"

bool bochan::AudioPlayer::initializeDefault(const char* audioDevice, int sampleRate) {
    this->sampleRate = sampleRate;
    size_t bytesPerSecond = CodecUtil::getBytesPerSecond(sampleRate);
    return initialize(audioDevice, sampleRate, bytesPerSecond / 2ULL, bytesPerSecond * 2ULL);
}

size_t bochan::AudioPlayer::getMinBufferSize() const {
    return minBufferSize;
}

size_t bochan::AudioPlayer::getMaxBufferSize() const {
    return maxBufferSize;
}
