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
        SignalProvider(SignalProvider&) = delete;
        SignalProvider(SignalProvider&&) = delete;
        SignalProvider& operator=(SignalProvider&) = delete;
        SignalProvider& operator=(SignalProvider&&) = delete;
        BOCHANAPI bool initialize(int sampleRate);
        BOCHANAPI void deinitialize() override;
        BOCHANAPI bool isInitialized() const override;
        BOCHANAPI bool fillBuffer(gsl::not_null<ByteBuffer*> buff) override;
        BOCHANAPI void setFrequency(double freq);
        BOCHANAPI double getFrequency() const;
        BOCHANAPI void setAmplitude(double amplitude);
        BOCHANAPI double getAmplitude() const;
        BOCHANAPI void setSignalWave(SignalWave signalWave);
        BOCHANAPI SignalWave getSignalWave() const;
        BOCHANAPI void setSimulateTime(bool simulateTime);
        BOCHANAPI bool isSimulatingTime() const;
    private:
        int sampleRate{ 0 };
        BufferPool* bufferPool{ nullptr };
        double frequency{ 440.0 };
        double amplitude{ 1.0 };
        double time{ 0.0 };
        bool startPointAvailable{ false }, simulateTime{ true };
        std::chrono::system_clock::time_point startPoint{};
        SignalWave signalWave{ SignalWave::Sin };
    };
}

