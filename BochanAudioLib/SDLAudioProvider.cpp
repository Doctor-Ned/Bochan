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

bool bochan::SDLAudioProvider::initialize(gsl::cstring_span audioDevice, int sampleRate, size_t bufferSize, bool forceMono) {
    if (initialized) {
        deinitialize();
    }
    if (!audioDevice.empty()) {
        BOCHAN_DEBUG("Initializing audio device '{}' with {} SR, {} buffer size (mono: {})...", audioDevice.cbegin(), sampleRate, bufferSize, forceMono);
    } else {
        BOCHAN_DEBUG("Initializing default audio device with {} SR, {} buffer size (mono: {})...", sampleRate, bufferSize, forceMono);
    }
    this->forceMono = forceMono;
    this->audioDevice = audioDevice;
    this->sampleRate = sampleRate;
    SDL_AudioSpec wanted{};
    wanted.freq = sampleRate;
    wanted.format = AUDIO_S16;
    wanted.channels = forceMono ? 1 : CodecUtil::CHANNELS;
    wanted.samples = 512;
    wanted.callback = audioCallback;
    wanted.userdata = this;
    if (devId = SDL_OpenAudioDevice(audioDevice.cbegin(), SDL_TRUE, &wanted, nullptr, 0); devId == 0) {
        if (!audioDevice.empty()) {
            BOCHAN_ERROR("Failed to open device '{}': {}", audioDevice.cbegin(), SDL_GetError());
        } else {
            BOCHAN_ERROR("Failed to open default audio device: {}", SDL_GetError());
        }
        return false;
    }
    sampleBuffer = gsl::make_span<uint8_t>(new uint8_t[bufferSize], bufferSize);
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
    forceMono = false;
    sampleRate = 0;
    delete[] sampleBuffer.begin();
    sampleBuffer = {};
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
        memcpy(ptr, sampleBuffer.begin(), remaining);
        reduceBuffer(remaining);
        return true;
    }
    while (remaining && initialized) {
        if (sampleBufferPos) {
            std::lock_guard lock(bufferMutex);
            size_t toRead = min(static_cast<size_t>(sampleBufferPos), remaining);
            memcpy(ptr, sampleBuffer.begin(), toRead);
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

std::vector<gsl::cstring_span> bochan::SDLAudioProvider::getAvailableDevices() const {
    std::vector<gsl::cstring_span> result{};
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
        memmove(sampleBuffer.begin(), sampleBuffer.begin() + size, sampleBufferPos - size);
    }
    sampleBufferPos -= size;
}

void bochan::SDLAudioProvider::audioCallback(void* ptr, Uint8* stream, int len) {
    SDLAudioProvider* provider = reinterpret_cast<SDLAudioProvider*>(ptr);
    std::lock_guard lock(provider->bufferMutex);
    const size_t CHANNELS = CodecUtil::CHANNELS;
    if (provider->forceMono && CHANNELS != 1) {
        if (provider->sampleBuffer.size_bytes() - provider->sampleBufferPos < CHANNELS * len) {
            size_t remaining{ provider->sampleBufferPos + CHANNELS * len - provider->sampleBuffer.size_bytes() };
            provider->reduceBuffer(remaining);
        }
        int samples = len / sizeof(int16_t);
        int16_t* streamPtr = reinterpret_cast<int16_t*>(stream);
        int16_t* buffPtr = reinterpret_cast<int16_t*>(provider->sampleBuffer.begin() + provider->sampleBufferPos);
        for (int i = 0; i < samples; ++i) {
            for (int j = 0; j < CHANNELS; ++j) {
                buffPtr[i * CHANNELS + j] = streamPtr[i];
            }
        }
        provider->sampleBufferPos += CHANNELS * len;
    } else {
        if (provider->sampleBuffer.size_bytes() - provider->sampleBufferPos < len) {
            size_t remaining{ provider->sampleBufferPos + len - provider->sampleBuffer.size_bytes() };
            provider->reduceBuffer(remaining);
        }
        memcpy(provider->sampleBuffer.begin() + provider->sampleBufferPos, stream, len);
        provider->sampleBufferPos += len;
    }
}
