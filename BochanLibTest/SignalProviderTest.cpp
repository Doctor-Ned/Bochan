#include "pch.h"
#include "SignalProvider.h"
#include "CodecUtil.h"

using namespace bochan;

constexpr int SAMPLE_RATE = 48000;

TEST(SignalProvider, TimeSimulatedProperly) {
    using namespace std::chrono;
    const milliseconds EXPECTED_MIN_MILLIS{ 950 };
    BufferPool bufferPool(1024 * 1024 * 1024);
    SignalProvider provider(bufferPool);
    ASSERT_TRUE(provider.initialize(SAMPLE_RATE));
    ASSERT_TRUE(provider.isSimulatingTime());
    ByteBuffer* buff{ bufferPool.getBuffer(CodecUtil::CHANNELS * SAMPLE_RATE * sizeof(int16_t)) };
    system_clock::time_point startPoint{ system_clock::now() }, endPoint;
    ASSERT_TRUE(provider.fillBuffer(buff));
    endPoint = system_clock::now();
    ASSERT_TRUE(duration_cast<milliseconds>(endPoint - startPoint) >= EXPECTED_MIN_MILLIS);
    startPoint = endPoint;
    ASSERT_TRUE(provider.fillBuffer(buff));
    endPoint = system_clock::now();
    ASSERT_TRUE(duration_cast<milliseconds>(endPoint - startPoint) >= EXPECTED_MIN_MILLIS);
    provider.setSimulateTime(false);
    ASSERT_FALSE(provider.isSimulatingTime());
    startPoint = endPoint;
    ASSERT_TRUE(provider.fillBuffer(buff));
    endPoint = system_clock::now();
    ASSERT_TRUE(duration_cast<milliseconds>(endPoint - startPoint) < EXPECTED_MIN_MILLIS);

}

TEST(SignalProvider, AmplitudeValid) {
    BufferPool bufferPool(1024 * 1024 * 1024);
    SignalProvider provider(bufferPool);
    ASSERT_TRUE(provider.initialize(SAMPLE_RATE));
    ByteBuffer* buff{ bufferPool.getBuffer(CodecUtil::CHANNELS * SAMPLE_RATE * sizeof(int16_t)) };
    provider.setSimulateTime(false);
    ASSERT_FALSE(provider.isSimulatingTime());
    ASSERT_EQ(provider.getAmplitude(), 1.0);
    provider.setAmplitude(1.0);
    ASSERT_EQ(provider.getAmplitude(), 1.0);
    int min{ 0 }, max{ 0 };
    int16_t* int16Ptr{ reinterpret_cast<int16_t*>(buff->getPointer()) };
    size_t samples{ buff->getUsedSize() / sizeof(int16_t) };
    ASSERT_TRUE(provider.fillBuffer(buff));
    for (size_t i = 0; i < samples; ++i) {
        if (int16Ptr[i] < min) {
            min = int16Ptr[i];
        }
        if (int16Ptr[i] > max) {
            max = int16Ptr[i];
        }
    }
    ASSERT_LE(max, 32767);
    ASSERT_GE(min, -32768);
    min = 0;
    max = 0;
    provider.setAmplitude(0.5);
    ASSERT_EQ(provider.getAmplitude(), 0.5);
    ASSERT_TRUE(provider.fillBuffer(buff));
    for (size_t i = 0; i < samples; ++i) {
        if (int16Ptr[i] < min) {
            min = int16Ptr[i];
        }
        if (int16Ptr[i] > max) {
            max = int16Ptr[i];
        }
    }
    ASSERT_LE(max, 16383);
    ASSERT_GE(min, -16384);
    bufferPool.freeBuffer(buff);
}