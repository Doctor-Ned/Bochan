#include "pch.h"
#include "SignalProvider.h"
#include "CodecUtil.h"

bochan::SignalProvider::SignalProvider(BufferPool& bufferPool) : bufferPool(&bufferPool) {}

BOCHANAPI bool bochan::SignalProvider::initialize(int sampleRate) {
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
    ByteBuffer* floatBuff = bufferPool->getBuffer(buff->getUsedSize() * sizeof(float) / sizeof(int16_t));
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
        float fltVal = static_cast<float>(sampleValue * amplitude);
        for (int j = 0; j < CodecUtil::CHANNELS; ++j) {
            floatPtr[i * CodecUtil::CHANNELS + j] = fltVal;
        }
    }
    CodecUtil::floatToInt16(floatPtr, samples * CodecUtil::CHANNELS, reinterpret_cast<int16_t*>(buff->getPointer()));
    bufferPool->freeBuffer(floatBuff);
    return true;
}

void bochan::SignalProvider::setFrequency(double freq) {
    this->frequency = freq;
}

double bochan::SignalProvider::getFrequency() const {
    return frequency;
}

void bochan::SignalProvider::setAmplitude(double amplitude) {
    this->amplitude = std::clamp(amplitude, 0.0, 1.0);
}

double bochan::SignalProvider::getAmplitude() const {
    return amplitude;
}

void bochan::SignalProvider::setSignalWave(SignalWave signalWave) {
    this->signalWave = signalWave;
}

BOCHANAPI bool bochan::SignalProvider::isSimulatingTime() const {
    return simulateTime;
}

bochan::SignalWave bochan::SignalProvider::getSignalWave() const {
    return signalWave;
}

BOCHANAPI void bochan::SignalProvider::setSimulateTime(bool simulateTime) {
    this->simulateTime = simulateTime;
    startPointAvailable = false;
}
