#include "pch.h"
#include "SDLAudioProvider.h"
#include "CodecUtil.h"
#include "SDLUtil.h"

bochan::SDLAudioProvider::SDLAudioProvider() {
    SDLUtil::initAudio(this);
}

bochan::SDLAudioProvider::~SDLAudioProvider() {
    if (initialized) {
        deinitialize();
    }
    SDLUtil::quitAudio(this);
}

bool bochan::SDLAudioProvider::initialize(const char* audioDevice, int sampleRate, size_t bufferSize, bool forceMono) {
    if (initialized) {
        deinitialize();
    }
    if (audioDevice) {
        BOCHAN_DEBUG("Initializing audio device '{}' with {} SR, {} buffer size (mono: {})...", audioDevice, sampleRate, bufferSize, forceMono);
    } else {
        BOCHAN_DEBUG("Initializing default audio device with {} SR, {} buffer size (mono: {})...", sampleRate, bufferSize, forceMono);
    }
    this->forceMono = forceMono;
    this->audioDevice = audioDevice;
    this->sampleRate = sampleRate;
    this->bufferSize = bufferSize;
    SDL_AudioSpec wanted{};
    wanted.freq = sampleRate;
    wanted.format = AUDIO_S16;
    wanted.channels = forceMono ? 1 : CodecUtil::CHANNELS;
    wanted.samples = 512;
    wanted.callback = audioCallback;
    wanted.userdata = this;
    if (devId = SDL_OpenAudioDevice(audioDevice, SDL_TRUE, &wanted, nullptr, 0); devId == 0) {
        if (audioDevice) {
            BOCHAN_ERROR("Failed to open device '{}': {}", audioDevice, SDL_GetError());
        } else {
            BOCHAN_ERROR("Failed to open default audio device: {}", SDL_GetError());
        }
        return false;
    }
    sampleBuffer = new uint8_t[bufferSize];
    sampleBufferPos = 0ULL;
    initialized = true;
    return true;
}

void bochan::SDLAudioProvider::deinitialize() {
    initialized = false;
    if (devId != 0U) {
        if (recording) {
            SDL_PauseAudioDevice(devId, SDL_TRUE);
        }
        recording = false;
        SDL_CloseAudioDevice(devId);
    }
    devId = 0U;
    audioDevice = nullptr;
    bufferSize = 0ULL;
    forceMono = false;
    sampleRate = 0;
    delete[] sampleBuffer;
    sampleBuffer = nullptr;
    sampleBufferPos = 0ULL;
}

bool bochan::SDLAudioProvider::isInitialized() const {
    return initialized;
}

bool bochan::SDLAudioProvider::isRecording() const {
    return recording;
}

bool bochan::SDLAudioProvider::record() {
    if (recording) {
        return true;
    }
    if (initialized) {
        SDL_PauseAudioDevice(devId, SDL_FALSE);
        recording = true;
        return true;
    }
    return false;
}

void bochan::SDLAudioProvider::stop() {
    if (initialized && recording) {
        SDL_PauseAudioDevice(devId, SDL_TRUE);
        recording = false;
    }
}

void bochan::SDLAudioProvider::flush() {
    if (initialized) {
        std::lock_guard lock(bufferMutex);
        sampleBufferPos = 0ULL;
    }
}

bool bochan::SDLAudioProvider::fillBuffer(gsl::not_null<ByteBuffer*> buff) {
    uint8_t* ptr = buff->getPointer();
    size_t remaining = buff->getUsedSize();
    if (!recording) {
        std::lock_guard lock(bufferMutex);
        if (sampleBufferPos < remaining) {
            BOCHAN_WARN("Failed to fill the buffer (not recording and not enough bytes buffered)");
            return false;
        }
        memcpy(ptr, sampleBuffer, remaining);
        reduceBuffer(remaining);
        return true;
    }
    while (remaining && initialized) {
        if (sampleBufferPos) {
            std::lock_guard lock(bufferMutex);
            size_t toRead = min(static_cast<size_t>(sampleBufferPos), remaining);
            memcpy(ptr, sampleBuffer, toRead);
            reduceBuffer(toRead);
            ptr += toRead;
            remaining -= toRead;
        }
    }
    if (remaining) { // stopped because it was deinitialized during read
        BOCHAN_WARN("Filling buffer stopped (deinitialization)");
        return false;
    }
    return true;
}

std::vector<const char*> bochan::SDLAudioProvider::getAvailableDevices() const {
    std::vector<const char*> result{};
    const int DEVICE_COUNT{ SDL_GetNumAudioDevices(SDL_TRUE) };
    for (int i = 0; i < DEVICE_COUNT; ++i) {
        result.push_back(SDL_GetAudioDeviceName(i, SDL_TRUE));
    }
    return result;
}

void bochan::SDLAudioProvider::reduceBuffer(size_t size) {
    assert(sampleBufferPos >= size);
    std::lock_guard lock(bufferMutex);
    if (sampleBufferPos > size) {
        memmove(sampleBuffer, sampleBuffer + size, sampleBufferPos - size);
    }
    sampleBufferPos -= size;
}

void bochan::SDLAudioProvider::audioCallback(void* ptr, Uint8* stream, int len) {
    SDLAudioProvider* provider = reinterpret_cast<SDLAudioProvider*>(ptr);
    std::lock_guard lock(provider->bufferMutex);
    const size_t CHANNELS = CodecUtil::CHANNELS;
    if (provider->forceMono && CHANNELS != 1) {
        if (provider->bufferSize - provider->sampleBufferPos < CHANNELS * len) {
            size_t remaining{ provider->sampleBufferPos + CHANNELS * len - provider->bufferSize };
            provider->reduceBuffer(remaining);
        }
        int samples = len / sizeof(int16_t);
        int16_t* streamPtr = reinterpret_cast<int16_t*>(stream);
        int16_t* buffPtr = reinterpret_cast<int16_t*>(provider->sampleBuffer + provider->sampleBufferPos);
        for (int i = 0; i < samples; ++i) {
            for (int j = 0; j < CHANNELS; ++j) {
                buffPtr[i * CHANNELS + j] = streamPtr[i];
            }
        }
        provider->sampleBufferPos += CHANNELS * len;
    } else {
        if (provider->bufferSize - provider->sampleBufferPos < len) {
            size_t remaining{ provider->sampleBufferPos + len - provider->bufferSize };
            provider->reduceBuffer(remaining);
        }
        memcpy(provider->sampleBuffer + provider->sampleBufferPos, stream, len);
        provider->sampleBufferPos += len;
    }
}
