#include "pch.h"
#include "SDLAudioPlayer.h"
#include "CodecUtil.h"
#include "SDLUtil.h"

bochan::SDLAudioPlayer::SDLAudioPlayer() {
    SDLUtil::initAudio(this);
}

bochan::SDLAudioPlayer::~SDLAudioPlayer() {
    if (initialized) {
        deinitialize();
    }
    SDLUtil::quitAudio(this);
}

bool bochan::SDLAudioPlayer::initialize(const char* audioDevice, int sampleRate, size_t minBufferSize, size_t maxBufferSize) {
    if (initialized) {
        deinitialize();
    }
    if (audioDevice) {
        BOCHAN_DEBUG("Initializing audio device '{}' with {} SR, {}/{} min/max buffer size...", audioDevice, sampleRate, minBufferSize, maxBufferSize);
    } else {
        BOCHAN_DEBUG("Initializing default audio device with {} SR, {}/{} min/max buffer size...", sampleRate, minBufferSize, maxBufferSize);
    }
    this->audioDevice = audioDevice;
    this->sampleRate = sampleRate;
    this->minBufferSize = minBufferSize;
    this->maxBufferSize = maxBufferSize;
    SDL_AudioSpec wanted{};
    wanted.freq = sampleRate;
    wanted.format = AUDIO_S16;
    wanted.channels = CodecUtil::CHANNELS;
    wanted.samples = 1024;
    wanted.callback = fillData;
    wanted.userdata = this;
    if (minBufferSize > maxBufferSize || minBufferSize < wanted.samples) {
        BOCHAN_ERROR("Invalid min/max buffer size ({}/{})!", minBufferSize, maxBufferSize);
        return false;
    }
    if (devId = SDL_OpenAudioDevice(audioDevice, SDL_FALSE, &wanted, nullptr, 0); devId == 0) {
        if (audioDevice) {
            BOCHAN_ERROR("Failed to open device '{}': {}", audioDevice, SDL_GetError());
        } else {
            BOCHAN_ERROR("Failed to open default audio device: {}", SDL_GetError());
        }
        return false;
    }
    sampleBuffer = new uint8_t[maxBufferSize];
    sampleBufferPos = 0ULL;
    initialized = true;
    return true;
}

void bochan::SDLAudioPlayer::deinitialize() {
    initialized = false;
    if (devId != 0U) {
        if (playing) {
            SDL_PauseAudioDevice(devId, SDL_TRUE);
        }
        playing = false;
        SDL_CloseAudioDevice(devId);
    }
    devId = 0U;
    audioDevice = nullptr;
    minBufferSize = maxBufferSize = 0ULL;
    sampleRate = 0;
    delete[] sampleBuffer;
    sampleBuffer = nullptr;
    sampleBufferPos = 0ULL;
}

bool bochan::SDLAudioPlayer::isInitialized() const {
    return initialized;
}

size_t bochan::SDLAudioPlayer::queueData(ByteBuffer* buff) {
    if (!initialized) {
        return 0ULL;
    }
    std::lock_guard lock(bufferMutex);
    size_t queued = min(maxBufferSize - sampleBufferPos, buff->getUsedSize());
    memcpy(sampleBuffer + sampleBufferPos, buff->getPointer(), queued);
    sampleBufferPos += queued;
    return queued;
}

bool bochan::SDLAudioPlayer::isPlaying() {
    return playing;
}

bool bochan::SDLAudioPlayer::play() {
    if (playing) {
        return true;
    }
    if (initialized && sampleBufferPos >= minBufferSize) {
        BOCHAN_DEBUG("Playback started!");
        SDL_PauseAudioDevice(devId, SDL_FALSE);
        playing = true;
        return true;
    }
    return false;
}

void bochan::SDLAudioPlayer::stop() {
    if (initialized && playing) {
        BOCHAN_DEBUG("Playback stopped!");
        SDL_PauseAudioDevice(devId, SDL_TRUE);
        playing = false;
    }
}

void bochan::SDLAudioPlayer::flush() {
    if (initialized) {
        std::lock_guard lock(bufferMutex);
        sampleBufferPos = 0ULL;
    }
}

std::vector<const char*> bochan::SDLAudioPlayer::getAvailableDevices() const {
    std::vector<const char*> result{};
    const int DEVICE_COUNT{ SDL_GetNumAudioDevices(SDL_FALSE) };
    for (int i = 0; i < DEVICE_COUNT; ++i) {
        result.push_back(SDL_GetAudioDeviceName(i, SDL_FALSE));
    }
    return result;
}

void bochan::SDLAudioPlayer::fillData(void* ptr, Uint8* stream, int len) {
    SDLAudioPlayer* player{ reinterpret_cast<SDLAudioPlayer*>(ptr) };
    std::lock_guard lock(player->bufferMutex);
    if (len > player->sampleBufferPos) {
        player->stop();
    } else {
        memcpy(stream, player->sampleBuffer, len);
        if (player->sampleBufferPos > len) {
            memmove(player->sampleBuffer, player->sampleBuffer + len, player->sampleBufferPos - len);
        }
        player->sampleBufferPos -= len;
    }
}
