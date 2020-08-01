#include "pch.h"
#include "BochanAudioPlayer.h"
#include "CodecUtil.h"

bochan::BochanAudioPlayer::~BochanAudioPlayer() {
    if (isInitialized()) {
        deinitialize();
    }
}

void bochan::BochanAudioPlayer::fillData(void* ptr, Uint8* stream, int len) {

}

bool bochan::BochanAudioPlayer::init(int sampleRate, size_t minBufferSize, size_t maxBufferSize) {
    SDL_AudioSpec wanted{};
    wanted.freq = sampleRate;
    wanted.format = AUDIO_S16;
    wanted.channels = CodecUtil::CHANNELS;
    wanted.samples = 1024;
    wanted.callback = fillData;
    wanted.userdata = this;
    if (SDL_OpenAudio(&wanted, nullptr) != 0) {
        BOCHAN_ERROR("Failed to open audio device: {}", SDL_GetError());
        return false;
    }
    return true;
}

void bochan::BochanAudioPlayer::deinitialize() {}

bool bochan::BochanAudioPlayer::isInitialized() const {
    return false;
}

bool bochan::BochanAudioPlayer::queueData(ByteBuffer* buff) {
    return false;
}

bool bochan::BochanAudioPlayer::isPlaying() {
    return false;
}

void bochan::BochanAudioPlayer::play() {}

void bochan::BochanAudioPlayer::stop() {}

void bochan::BochanAudioPlayer::flush() {}
