#include "pch.h"
#include "AudioPlayer.h"
#include "CodecUtil.h"

bool bochan::AudioPlayer::init(int sampleRate) {
    size_t bytesPerSecond{ static_cast<size_t>(sampleRate) * CodecUtil::CHANNELS * sizeof(int16_t) };
    return init(sampleRate, bytesPerSecond / 2ULL, bytesPerSecond * 2ULL);
}
