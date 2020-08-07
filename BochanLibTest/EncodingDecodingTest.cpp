#include "pch.h"
#include "SignalProvider.h"
#include "BochanEncoder.h"
#include "BochanDecoder.h"

using namespace bochan;

TEST(EncodingDecoding, BitrateBasedQualityLossTest) {
    const float EXPECTED_HQ_ERR{ 0.05f }, EXPECTED_LQ_ERR{ 0.40f };
    const size_t FRAME_COUNT{ 256ULL };
    const int SAMPLE_RATE{ 48000 };
    const CodecConfig HQ_CONFIG{ BochanCodec::Opus, SAMPLE_RATE, 196000 }, LQ_CONFIG{ BochanCodec::Opus, SAMPLE_RATE, 4000 };
    BufferPool bufferPool(1024 * 1024 * 1024);
    SignalProvider provider(bufferPool);
    provider.setSimulateTime(false);
    BochanEncoder hqEncoder(bufferPool), lqEncoder(bufferPool);
    BochanDecoder hqDecoder(bufferPool), lqDecoder(bufferPool);
    ASSERT_TRUE(provider.initialize(SAMPLE_RATE));
    ASSERT_TRUE(hqEncoder.initialize(HQ_CONFIG));
    ASSERT_TRUE(hqDecoder.initialize(HQ_CONFIG, false, hqDecoder.needsExtradata(HQ_CONFIG.codec) ? hqEncoder.getExtradata() : nullptr));
    ASSERT_TRUE(lqEncoder.initialize(LQ_CONFIG));
    ASSERT_TRUE(lqDecoder.initialize(LQ_CONFIG, false, lqDecoder.needsExtradata(LQ_CONFIG.codec) ? lqEncoder.getExtradata() : nullptr));
    ASSERT_EQ(hqEncoder.getInputBufferByteSize(), lqEncoder.getInputBufferByteSize());
    ByteBuffer* inputSignal{ bufferPool.getBuffer(hqEncoder.getInputBufferByteSize() * FRAME_COUNT) };
    ByteBuffer* hqSignal{ bufferPool.getBuffer(inputSignal->getUsedSize()) }, * lqSignal{ bufferPool.getBuffer(inputSignal->getUsedSize()) };
    ASSERT_TRUE(provider.fillBuffer(inputSignal));
    ByteBuffer* frameBuff{ bufferPool.getBuffer(hqEncoder.getInputBufferByteSize()) };
    size_t hqPos{ 0ULL }, lqPos{ 0ULL };
    for (size_t i = 0ULL; i < FRAME_COUNT; ++i) {
        memcpy(frameBuff->getPointer(), inputSignal->getPointer() + i * frameBuff->getUsedSize(), frameBuff->getUsedSize());
        for (AudioPacket packet : hqEncoder.encode(frameBuff)) {
            for (ByteBuffer* dec : hqDecoder.decode(packet)) {
                memcpy(hqSignal->getPointer() + hqPos, dec->getPointer(), dec->getUsedSize());
                hqPos += dec->getUsedSize();
                bufferPool.freeBuffer(dec);
            }
            bufferPool.freeBuffer(packet.buffer);
        }
        for (AudioPacket packet : lqEncoder.encode(frameBuff)) {
            for (ByteBuffer* dec : lqDecoder.decode(packet)) {
                memcpy(lqSignal->getPointer() + lqPos, dec->getPointer(), dec->getUsedSize());
                lqPos += dec->getUsedSize();
                bufferPool.freeBuffer(dec);
            }
            bufferPool.freeBuffer(packet.buffer);
        }
    }
    size_t sizeToCompare{ min(hqPos, lqPos) };
    size_t samples{ sizeToCompare / sizeof(int16_t) };
    float* inputFlt{ new float[samples] }, * hqFlt{ new float[samples] }, * lqFlt{ new float[samples] };
    CodecUtil::int16ToFloat(reinterpret_cast<int16_t*>(inputSignal->getPointer()), samples, inputFlt);
    CodecUtil::int16ToFloat(reinterpret_cast<int16_t*>(hqSignal->getPointer()), samples, hqFlt);
    CodecUtil::int16ToFloat(reinterpret_cast<int16_t*>(lqSignal->getPointer()), samples, lqFlt);
    float hqErr{ 0.0f }, lqErr{ 0.0f };
    for (size_t i = 0ULL; i < samples; ++i) {
        hqErr += abs(hqFlt[i] - inputFlt[i]);
        lqErr += abs(lqFlt[i] - inputFlt[i]);
    }
    hqErr /= static_cast<float>(samples);
    lqErr /= static_cast<float>(samples);
    ASSERT_LE(hqErr, EXPECTED_HQ_ERR);
    ASSERT_LE(lqErr, EXPECTED_LQ_ERR);
    ASSERT_LE(hqErr, lqErr);
    BOCHAN_DEBUG("Produced {} frames and tested {} samples. HQ codec error: {}%, LQ codec error: {}%.",
                 FRAME_COUNT, samples, hqErr * 100.0f, lqErr * 100.0f);
    delete[] inputFlt;
    delete[] hqFlt;
    delete[] lqFlt;
    bufferPool.freeBuffer(inputSignal);
    bufferPool.freeBuffer(hqSignal);
    bufferPool.freeBuffer(lqSignal);
    bufferPool.freeBuffer(frameBuff);
}