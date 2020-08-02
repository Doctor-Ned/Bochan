#include "pch.h"
#include "SignalProvider.h"
#include "CodecUtil.h"

bochan::SignalProvider::SignalProvider(BufferPool& bufferPool) : bufferPool(&bufferPool) {}

bool bochan::SignalProvider::init(int sampleRate) {
    this->sampleRate = sampleRate;
    return true;
}

void bochan::SignalProvider::deinitialize() {
    sampleRate = 0;
    time = 0.0;
    startPointAvailable = false;
}

bool bochan::SignalProvider::isInitialized() const {
    return sampleRate != 0;
}

bool bochan::SignalProvider::fillBuffer(ByteBuffer* buff) {
    if (sampleRate == 0) {
        return false;
    }
    if (startPointAvailable) {
        std::this_thread::sleep_until(startPoint);
    } else {
        startPointAvailable = true;
        startPoint = std::chrono::system_clock::now();
    }
    int samples = static_cast<int>(buff->getUsedSize()) / sizeof(uint16_t) / CodecUtil::CHANNELS;
    std::chrono::microseconds buffTimeNanos{
        static_cast<long long>(
            floor(1'000'000.0 / static_cast<double>(sampleRate))
            * static_cast<double>(samples)
            ) };
    ByteBuffer* floatBuff = bufferPool->getBuffer(buff->getUsedSize() * 2ULL);
    float* floatPtr = reinterpret_cast<float*>(floatBuff->getPointer());
    double tincr = 2.0 * M_PI * frequency / static_cast<double>(sampleRate);
    for (int i = 0; i < samples; ++i) {
        double sampleValue = sin(time);
        time += tincr;
        switch (signalWave) {
            case SignalWave::Square:
                if (sampleValue != 0.0) {
                    sampleValue = sampleValue > 0.0 ? 1.0 : -1.0;
                }
                break;
        }
        float fltVal = static_cast<float>(sampleValue);
        for (int j = 0; j < CodecUtil::CHANNELS; ++j) {
            floatPtr[i * CodecUtil::CHANNELS + j] = fltVal;
        }
    }
    CodecUtil::floatToInt16(floatPtr, samples * CodecUtil::CHANNELS, reinterpret_cast<int16_t*>(buff->getPointer()));
    bufferPool->freeBuffer(floatBuff);
    startPoint += buffTimeNanos;
    return true;
}

void bochan::SignalProvider::setFrequency(double freq) {
    this->frequency = freq;
}

double bochan::SignalProvider::getFrequency() const {
    return frequency;
}

void bochan::SignalProvider::setSignalWave(SignalWave signalWave) {
    this->signalWave = signalWave;
}

bochan::SignalWave bochan::SignalProvider::getSignalWave() const {
    return signalWave;
}
