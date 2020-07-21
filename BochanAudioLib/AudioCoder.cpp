#include "pch.h"
#include "AudioCoder.h"

bochan::AudioCoder::AudioCoder(BufferPool* bufferPool) : bufferPool(bufferPool) {}

bochan::ByteBuffer* bochan::AudioCoder::samplesToFloat(ByteBuffer* samples) {
    ByteBuffer* result = bufferPool->getBuffer(samples->getSize() * sizeof(float) / sizeof(int16_t));
    float* floatBuff = reinterpret_cast<float*>(result->getPointer());
    int16_t* int16Buff = reinterpret_cast<int16_t*>(samples->getPointer());
    size_t sampleCount = samples->getSize() / sizeof(int16_t);
    for (int i = 0; i < sampleCount; ++i) {
        floatBuff[i] = static_cast<float>(int16Buff[i]) / 32768.0f;
    }
    return result;
}

bochan::ByteBuffer* bochan::AudioCoder::floatToSamples(ByteBuffer* fltp) {
    ByteBuffer* result = bufferPool->getBuffer(fltp->getSize() * sizeof(int16_t) / sizeof(float));
    float* floatBuff = reinterpret_cast<float*>(fltp->getPointer());
    int16_t* intBuff = reinterpret_cast<int16_t*>(result->getPointer());
    size_t sampleCount = fltp->getSize() / sizeof(float);
    for (int i = 0; i < sampleCount; ++i) {
        intBuff[i] = static_cast<int16_t>(floatBuff[i] * 32767.9f);
    }
    return result;
}
