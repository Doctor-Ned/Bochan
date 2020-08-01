#pragma once

#include "AudioProvider.h"
#include <chrono>

namespace bochan {
    enum class SignalWave {
        Sin,
        Square
    };

    class SignalProvider : public AudioProvider {
    public:
        BOCHANAPI SignalProvider(BufferPool& bufferPool);
        ~SignalProvider() = default;
        BOCHANAPI bool init(int sampleRate) override;
        BOCHANAPI void deinitialize() override;
        BOCHANAPI bool isInitialized() const override;
        BOCHANAPI bool fillBuffer(ByteBuffer* buff) override;
        BOCHANAPI void setFrequency(double freq);
        BOCHANAPI double getFrequency() const;
        BOCHANAPI void setSignalWave(SignalWave signalWave);
        BOCHANAPI SignalWave getSignalWave() const;
    private:
        BufferPool* bufferPool{ nullptr };
        double frequency{ 440.0 };
        double time{ 0.0 };
        bool startPointAvailable{ false };
        std::chrono::system_clock::time_point startPoint{};
        SignalWave signalWave{ SignalWave::Sin };
    };
}

